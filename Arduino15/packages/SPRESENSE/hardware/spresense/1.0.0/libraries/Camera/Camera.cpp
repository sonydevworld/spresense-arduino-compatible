/*
 *  Camera.cpp - Camera implementation file for the Spresense SDK
 *  Copyright 2018, 2020-2022 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file Camera.cpp
 * @author Sony Semiconductor Solutions Corporation
 * @brief Camera Library for Arduino IDE on Spresense.
 * @details Camera Classes for SPRESENSE Camera library.
 */

#include <string.h>
#include <fcntl.h>
#include <sched.h>
#include <errno.h>
#include <assert.h>

#include <Camera.h>
#include <arch/board/cxd56_imageproc.h>
#include <nuttx/video/isx012.h>
#include <nuttx/video/isx019.h>
#include <arch/chip/cisif.h>

/****************************************************************************
 * ImgBuff implementation.
 ****************************************************************************/
ImgBuff::ImgBuff()
  : ref_count(0), buff(NULL), width(0), height(0), idx(-1), is_queue(false),
    buf_type(V4L2_BUF_TYPE_VIDEO_CAPTURE), cam_ref(NULL)
{
}

ImgBuff::ImgBuff(enum v4l2_buf_type type,
                 int w, int h, CAM_IMAGE_PIX_FMT fmt, int jpgbufsize_divisor,
                 CameraClass *cam)
  : ref_count(0), buff(NULL), width(0), height(0), idx(-1), is_queue(false),
    buf_type(type), pix_fmt(CAM_IMAGE_PIX_FMT_NONE),
    buf_size(0), actual_size(0), cam_ref(NULL)
{
  buf_size = calc_img_size(w, h, fmt, jpgbufsize_divisor);
  if ((buf_size >= 1) && generate_imgmem(buf_size))
    {
      sem_init(&my_sem, 0, 1);
      cam_ref = cam;
      width = w;
      height = h;
      pix_fmt = fmt;
    }
}

ImgBuff::~ImgBuff()
{
  if (buff != NULL)
    {
      free(buff);
      sem_destroy(&my_sem);
      buff = NULL;
    }
}

bool ImgBuff::generate_imgmem(size_t s)
{
  if (s > 0)
    {
      // SPRESENSE Camera CamImage memory is needed 4 byte alignment.

      buff = (uint8_t *)memalign(SPRESENSE_CAMIMAGE_MEM_ALIGN, s);

      if (buff != NULL)
        {
          return true;
        }
    }

  return false;
}

void ImgBuff::incRef()
{
  lock();
  ref_count++;
  unlock();
}

bool ImgBuff::decRef()
{
  bool ret = false;
  lock();
  ref_count--;
  if (ref_count <= 0)
    {
      ret = true;
    }
  unlock();
  return ret;
}

void ImgBuff::delete_inst(ImgBuff *buf)
{
  if (buf != NULL)
    {
      if (buf->decRef())
        {
          if (buf->cam_ref != NULL)
            {
              buf->cam_ref->release_buf(buf);
            }
          else
            {
              delete buf;
            }
        }
    }
}

size_t ImgBuff::calc_img_size(int w, int h, CAM_IMAGE_PIX_FMT fmt, int jpgbufsize_divisor)
{
  size_t ret = -1;

  if ((w >= 1) && (h >= 1))
    {
      switch (fmt)
        {
          case CAM_IMAGE_PIX_FMT_RGB565:
          case CAM_IMAGE_PIX_FMT_YUV422:
            ret = w * h * 2;
            break;
          case CAM_IMAGE_PIX_FMT_JPG:
            ret = (size_t)(w * h * 2 / jpgbufsize_divisor);
            break;
          default:
            break;
        }
    }

  return ret;
}

void ImgBuff::update_actual_size(size_t sz)
{
  if (sz >= buf_size)
    {
      sz = buf_size;
    }
  actual_size = sz;
}

/****************************************************************************
 * CamImage implementation.
 ****************************************************************************/
CamImage::CamImage(const CamImage &obj)
{
  img_buff = obj.img_buff;
  img_buff->incRef();
}

CamImage &CamImage::operator=(const CamImage &obj)
{
  ImgBuff::delete_inst(img_buff);

  img_buff = obj.img_buff;
  img_buff->incRef();

  return (*this);
}

CamErr CamImage::convertPixFormat(CAM_IMAGE_PIX_FMT to_fmt)
{
  CAM_IMAGE_PIX_FMT from_fmt = getPixFormat();
  int               width    = getWidth();
  int               height   = getHeight();
  uint8_t           *buff    = getImgBuff();

  if (buff == NULL)
    {
      return CAM_ERR_NOT_PERMITTED;
    }

  switch (from_fmt)
    {
      case CAM_IMAGE_PIX_FMT_YUV422:
        switch (to_fmt)
          {
            case CAM_IMAGE_PIX_FMT_RGB565:
              imageproc_convert_yuv2rgb(buff, width, height);
              setPixFormat(to_fmt);
              break;

            case CAM_IMAGE_PIX_FMT_GRAY:
              imageproc_convert_yuv2gray(buff, buff, width, height);
              setActualSize(width * height);
              setPixFormat(to_fmt);
              break;

            default:
              return CAM_ERR_INVALID_PARAM;
          }

        break;

      case CAM_IMAGE_PIX_FMT_RGB565:
        switch (to_fmt)
          {
            case CAM_IMAGE_PIX_FMT_YUV422:
              imageproc_convert_rgb2yuv(buff, width, height);
              setPixFormat(to_fmt);
              break;

            case CAM_IMAGE_PIX_FMT_GRAY:
              imageproc_convert_rgb2yuv(buff, width, height);
              imageproc_convert_yuv2gray(buff, buff, width, height);
              setActualSize(width * height);
              setPixFormat(to_fmt);
              break;

            default:
              return CAM_ERR_INVALID_PARAM;
          }

        break;

      default:
        return CAM_ERR_INVALID_PARAM;
    }

  return CAM_ERR_SUCCESS;
}

// Check resize limitation of HW in GE2D of CXD5602.
#define IS_INVALID_SIZE(w, h) ((w) < 12 || (w) > 768 || (h) < 12 || (h) > 1024)

// Check resize magnification limitation.
// range can set as 1/2^n to 2^n (n=0..5)
bool CamImage::check_resize_magnification(int in, int out)
{
  unsigned int ratio;
  if(in > out)
    {
      ratio = in / out;
    }
  else
    {
      int tmp;
      ratio = out / in;

      // Swap "in" and "out".
      // Because "in" must be bigger than "out" for below logic.
      tmp = in;
      in = out;
      out = tmp;
    }

  unsigned int check_ratio = 64;
  while(check_ratio)
    {
      if( !(check_ratio ^ ratio) ) // Check ratio a power of 2.
        {
          // To check this, "in" must be bigger than "out".
          if( (out*ratio) == (unsigned int)in )
            {
              return true;
            }
          else
            {
              return false;
            }
        }
      check_ratio >>= 1;
    }
  return false;
}

#define CHECK_LSBONE(p) ((p)&0x01)

bool CamImage::check_hw_resize_param(int iw, int ih, int ow, int oh)
{
  // Size must be "Even number"
  if( CHECK_LSBONE(iw) || CHECK_LSBONE(ih) || CHECK_LSBONE(ow) || CHECK_LSBONE(oh) )
    {
      return false;
    }

  // Check resize limitation.
  if(IS_INVALID_SIZE(iw, ih) || IS_INVALID_SIZE(ow, oh))
    {
      return false;
    }

  // Check resize magnification limitation.
  return    check_resize_magnification(iw, ow)
         && check_resize_magnification(ih, oh);
}


CamErr CamImage::resizeImageByHW(CamImage &img, int width, int height)
{
  // Input instance must not be Capture Frames.
  if((img.is_valid()) && (img.img_buff->cam_ref != NULL))
    {
      return CAM_ERR_INVALID_PARAM;
    }

  // Format check.
  if( getPixFormat() != CAM_IMAGE_PIX_FMT_YUV422 )
    {
      return CAM_ERR_INVALID_PARAM;
    }

  // HW limitation check.
  if( !check_hw_resize_param( img_buff->width, img_buff->height, width, height ) )
    {
      return CAM_ERR_INVALID_PARAM;
    }

  CamImage *tmp_img = new CamImage(V4L2_BUF_TYPE_VIDEO_CAPTURE, width, height, getPixFormat());
  if( tmp_img == NULL || !tmp_img->is_valid() )
    {
      if(tmp_img != NULL) delete tmp_img;
      return CAM_ERR_NO_MEMORY;
    }
  tmp_img->setActualSize(tmp_img->img_buff->buf_size);

  // Execute resizing.
  int ret = imageproc_resize(getImgBuff(), getWidth(), getHeight(),
                  tmp_img->getImgBuff(), tmp_img->getWidth(), tmp_img->getHeight(), 16);
  if( ret != 0 )
    {
      delete tmp_img;
      return CAM_ERR_ILLEGAL_DEVERR;
    }

  // if the image has image buffer, delete it.
  if( img.is_valid() )
    {
      ImgBuff::delete_inst(img.img_buff);
    }

  // Set resized image buffer into input parameter.
  img.img_buff = tmp_img->img_buff;
  img.img_buff->incRef();

  tmp_img->img_buff = NULL;
  delete tmp_img;

  return CAM_ERR_SUCCESS;
}


CamErr CamImage::clipAndResizeImageByHW(
    CamImage &img,
    int lefttop_x,
    int lefttop_y,
    int rightbottom_x,
    int rightbottom_y,
    int width,
    int height)
{
  int clip_width, clip_height;
  imageproc_rect_t inrect;

  // Input instance must not be Capture Frames.
  if((img.is_valid()) && (img.img_buff->cam_ref != NULL))
    {
      return CAM_ERR_INVALID_PARAM;
    }

  // Format check.
  if( getPixFormat() != CAM_IMAGE_PIX_FMT_YUV422 )
    {
      return CAM_ERR_INVALID_PARAM;
    }

  clip_width  = rightbottom_x - lefttop_x + 1;
  clip_height = rightbottom_y - lefttop_y + 1;

  // Check clip area.
  if( (lefttop_x   < 0) || (lefttop_x  > rightbottom_x) ||
      (lefttop_y   < 0) || (lefttop_y  > rightbottom_y) ||
      (clip_width  < 0) || (clip_width  > getWidth())   ||
      (clip_height < 0) || (clip_height > getHeight()) )
    {
      return CAM_ERR_INVALID_PARAM;
    }

  // HW limitation check.
  if( !check_hw_resize_param( clip_width, clip_height, width, height ) )
    {
      return CAM_ERR_INVALID_PARAM;
    }

  CamImage *tmp_img = new CamImage(V4L2_BUF_TYPE_VIDEO_CAPTURE, width, height, getPixFormat());
  if( tmp_img == NULL || !tmp_img->is_valid() )
    {
      if(tmp_img != NULL) delete tmp_img;
      return CAM_ERR_NO_MEMORY;
    }
  tmp_img->setActualSize(tmp_img->img_buff->buf_size);

  inrect.x1 = lefttop_x;
  inrect.y1 = lefttop_y;
  inrect.x2 = rightbottom_x;
  inrect.y2 = rightbottom_y;

  // Execute clip and resize.
  int ret = imageproc_clip_and_resize(getImgBuff(), getWidth(), getHeight(),
                  tmp_img->getImgBuff(), tmp_img->getWidth(), tmp_img->getHeight(), 16, &inrect);
  if( ret != 0 )
    {
      delete tmp_img;
      return CAM_ERR_ILLEGAL_DEVERR;
    }

  // if the image has image buffer, delete it.
  if( img.is_valid() )
    {
      ImgBuff::delete_inst(img.img_buff);
    }

  // Set resized image buffer into input parameter.
  img.img_buff = tmp_img->img_buff;
  img.img_buff->incRef();

  tmp_img->img_buff = NULL;
  delete tmp_img;

  return CAM_ERR_SUCCESS;
}


bool CamImage::isAvailable(void)
{
   return (img_buff != NULL && img_buff->actual_size > 0);
}


CamImage::CamImage(enum v4l2_buf_type type,
                   int w, int h, CAM_IMAGE_PIX_FMT fmt, int jpgbufsize_divisor,
                   CameraClass *cam)
{
  img_buff = new ImgBuff(type, w, h, fmt, jpgbufsize_divisor, cam);

  if (!img_buff->is_valid())
    {
      delete img_buff;
      img_buff = NULL;
    }
}

CamImage::~CamImage()
{
  ImgBuff::delete_inst(img_buff);
}

/****************************************************************************
 * CameraClass implementation.
 ****************************************************************************/

#define VIDEO_DEV_FILE_NAME "/dev/video"

#define DELETE_CAMIMAGE(param_img) do {   \
  if((param_img)){                        \
    delete (param_img)->img_buff;         \
    (param_img)->img_buff = NULL;         \
    delete (param_img);                   \
    (param_img) = NULL;                   \
  }                                       \
}while(0)

// Private : Camera singleton instance.
CameraClass *CameraClass::instance = NULL;

// Public : singleton getter of Camera instance.
CameraClass CameraClass::getInstance()
{
  if (CameraClass::instance == NULL)
    {
      CameraClass::instance = new CameraClass(VIDEO_DEV_FILE_NAME);
    }

  return *CameraClass::instance;
}

#define STILL_STATUS_NO_INIT (0)
#define STILL_STATUS_QUEUED (1)
#define STILL_STATUS_TAKING (2)

// Public : Constructor.
CameraClass::CameraClass(const char *path)
{
  video_init_stat = isx019_initialize();
  video_init_stat += isx012_initialize();
  video_init_stat += cxd56_cisif_initialize();
  video_init_stat += video_initialize(path);
  video_fd = -1;
  video_imgs = NULL;
  video_buf_num = 0;
  video_pix_fmt = CAM_IMAGE_PIX_FMT_NONE;
  still_img = NULL;
  still_pix_fmt = CAM_IMAGE_PIX_FMT_NONE;
  loop_dqbuf_en = false;
  video_cb = NULL;
  dq_tid = -1;
  frame_tid = -1;
  sem_init(&video_cb_access_sem, 0, 1);
}

// Public : Destructor.
CameraClass::~CameraClass()
{
  video_uninitialize();
  CameraClass::instance = NULL;
}

// Private : Convert errno to camera error code.
CamErr CameraClass::convert_errno2camerr(int err)
{
  switch (err)
    {
      case ENODEV:
        return CAM_ERR_NO_DEVICE;

      case EPERM:
        return CAM_ERR_NOT_PERMITTED;

      case EINVAL:
        return CAM_ERR_INVALID_PARAM;

      case ENOMEM:
        return CAM_ERR_NO_MEMORY;

      default:
        return CAM_ERR_ILLEGAL_DEVERR;
    }
}

// Private : Set V4S frame paramters.
CamErr CameraClass::set_frame_parameters( enum v4l2_buf_type type, int video_width, int video_height, int buf_num, CAM_IMAGE_PIX_FMT video_fmt )
{
  struct v4l2_requestbuffers req = {0};
  struct v4l2_format fmt         = {0};

  // Set Buffer Mode.
  req.type = type;
  req.memory = V4L2_MEMORY_USERPTR;
  req.count = buf_num;
  req.mode = V4L2_BUF_MODE_RING; /* Freeze as ring mode. */
  if (ioctl(video_fd, VIDIOC_REQBUFS, (unsigned long)&req) < 0)
    {
      return convert_errno2camerr(errno);
    }

  // Set Format.
  fmt.type                = type;
  fmt.fmt.pix.width       = video_width;
  fmt.fmt.pix.height      = video_height;
  fmt.fmt.pix.field       = V4L2_FIELD_ANY;
  fmt.fmt.pix.pixelformat = video_fmt;
  if (ioctl(video_fd, VIDIOC_S_FMT, (unsigned long)&fmt) < 0)
    {
      return convert_errno2camerr(errno);
    }

  return CAM_ERR_SUCCESS;
}

// Private : Create Video frame buffers.
CamErr CameraClass::create_videobuff(int w, int h, int buff_num, CAM_IMAGE_PIX_FMT fmt,
                                     int jpgbufsize_divisor)
{
  int i;

  video_imgs = (CamImage **)malloc(sizeof(CamImage *) * buff_num);
  if (video_imgs == NULL)
    {
      return CAM_ERR_NO_MEMORY;
    }

  for (i = 0; i < buff_num; i++)
    {
      video_imgs[i]
       = new CamImage(V4L2_BUF_TYPE_VIDEO_CAPTURE, w, h, fmt, jpgbufsize_divisor, this);
      if ((video_imgs[i] == NULL) || !video_imgs[i]->is_valid())
        {
          if (video_imgs[i] != NULL)
            {
              delete video_imgs[i];
            }

          while (i > 0)
            {
              i--;
              DELETE_CAMIMAGE(video_imgs[i]);
            }
          delete video_imgs;
          return CAM_ERR_NO_MEMORY;
        }

      video_imgs[i]->setIdx(i);
    }

  video_buf_num = buff_num;

  return CAM_ERR_SUCCESS;
}

#define STILL_BUFF_IDX  (1000)

// Private : Create Still picture buffers.
CamErr CameraClass::create_stillbuff(int w, int h, CAM_IMAGE_PIX_FMT fmt,
                                     int jpgbufsize_divisor)
{
  if (still_img != NULL)
    {
      if (still_img->img_buff->is_queued())
        {
          ioctl(video_fd, VIDIOC_CANCEL_DQBUF, still_img->getType());
          DELETE_CAMIMAGE(still_img);
        }
      else
        {
          return CAM_ERR_USR_INUSED;
        }
    }

  still_img = new CamImage(V4L2_BUF_TYPE_STILL_CAPTURE, w, h, fmt,
                           jpgbufsize_divisor, this);

  if (still_img == NULL)
    {
      return CAM_ERR_NO_MEMORY;
    }
  else
    {
      if (!still_img->is_valid())
        {
          delete still_img;
          still_img = NULL;
          return CAM_ERR_NO_MEMORY;
        }
    }

  still_img->setIdx(STILL_BUFF_IDX);

  return CAM_ERR_SUCCESS;
}

// Private : Delete Video buffers.
void CameraClass::delete_videobuff()
{
  if (video_imgs){
    for (int i = 0; i < video_buf_num; i++)
      {
        if (video_imgs[i])
          {
            DELETE_CAMIMAGE(video_imgs[i]);
          }
      }
    delete video_imgs;
  }

  video_imgs = NULL;
  video_buf_num = 0;
}

// Private : Enqueue All Video buffes.
CamErr CameraClass::enqueue_video_buffs()
{
  CamErr err = CAM_ERR_SUCCESS;

  for (int i = 0; i < video_buf_num; i++)
    {
      err = enqueue_video_buff(video_imgs[i]);
      if (err != CAM_ERR_SUCCESS)
        {
          break;
        }
    }

  return err;
}

// Private : Enqueue One Video buffe.
CamErr CameraClass::enqueue_video_buff(CamImage * img)
{
  v4l2_buffer_t buf;

  memset(&buf, 0, sizeof(v4l2_buffer_t));
  buf.type = img->getType();
  buf.memory = V4L2_MEMORY_USERPTR;
  buf.index = img->getIdx();
  buf.m.userptr = (unsigned long)img->getImgBuff();
  buf.length = img->img_buff->buf_size;

  if (ioctl(video_fd, VIDIOC_QBUF, (unsigned long)&buf) < 0)
    {
      // No dequeue successed buffers.
      // because devie file "close" will clean up everthing.
      // TODO
      return convert_errno2camerr(errno);
    }

  img->img_buff->queued(true);

  return CAM_ERR_SUCCESS;
}

// Private : Check if device is ready to use.
bool CameraClass::is_device_ready()
{
  return (video_init_stat == 0) && (video_fd >= 0);
}

// Conversion table to parameter of time par frame of S_PARM
static struct fps_to_timeparframe
{
  CAM_VIDEO_FPS fps;
  uint32_t n;
  uint32_t d;
} fps2tpf[] = {
  {CAM_VIDEO_FPS_5,   1, 5},
  {CAM_VIDEO_FPS_6,   1, 6},
  {CAM_VIDEO_FPS_7_5, 2, 15},
  {CAM_VIDEO_FPS_15,  1, 15},
  {CAM_VIDEO_FPS_30,  1, 30},
  {CAM_VIDEO_FPS_60,  1, 60},
  {CAM_VIDEO_FPS_120, 1, 120},
};
#define SIZE_OF_TPF (sizeof(fps2tpf)/sizeof(fps2tpf[0]))

// Private : Video frame rate setting.
CamErr CameraClass::set_video_frame_rate(CAM_VIDEO_FPS fps)
{
  struct v4l2_streamparm param = {0};
  struct fps_to_timeparframe *tpf = NULL;

  for (unsigned int i=0; i < SIZE_OF_TPF; i++)
    {
      if (fps2tpf[i].fps == fps)
        {
          tpf = &fps2tpf[i];
        }
    }

  if (tpf == NULL)
    {
      return CAM_ERR_INVALID_PARAM;
    }

  // memset(&param, 0, sizeof(struct v4l2_streamparm));

  param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  param.parm.capture.timeperframe.numerator = tpf->n;
  param.parm.capture.timeperframe.denominator = tpf->d;

  if (ioctl(video_fd, VIDIOC_S_PARM, (unsigned long)&param) < 0)
    {
      return convert_errno2camerr(errno);
    }

  return CAM_ERR_SUCCESS;
}

#define CAM_FRAME_MQ_NAME "thecamera_mq"

CamErr CameraClass::create_dq_thread()
{
  CamErr err = CAM_ERR_SUCCESS;

  struct sched_param param;
  pthread_attr_t tattr;
  struct mq_attr mq_attr;

  mq_attr.mq_maxmsg  = CAM_FRAME_MQ_SIZE;
  mq_attr.mq_msgsize = sizeof(CamImage *);
  mq_attr.mq_flags   = 0;

  // Message queue for exchange frame buffer between dqbuf_thread and frame_handling_thread.
  frame_exchange_mq = mq_open(CAM_FRAME_MQ_NAME, (O_RDWR | O_CREAT), 0666, &mq_attr);
  if(frame_exchange_mq < 0)
    {
      return CAM_ERR_ILLEGAL_DEVERR;
    }

  // thread for callback to user operation.
  pthread_attr_init(&tattr);
  tattr.stacksize = CAM_FRAME_THREAD_STACK_SIZE;
  param.sched_priority = CAM_FRAME_THREAD_STACK_PRIO;
  pthread_attr_setschedparam(&tattr, &param);

  loop_dqbuf_en = true;
  if (pthread_create(
        &frame_tid,
        &tattr,
        (pthread_startroutine_t)CameraClass::frame_handle_thread,
        (void *)this))
    {
      loop_dqbuf_en = false;
      mq_close(frame_exchange_mq);
      return CAM_ERR_CANT_CREATE_THREAD;
    }
  else
    {
      pthread_setname_np(frame_tid, "frame_hdr_thread");
    }

  // thread for dqbuf from video driver.
  pthread_attr_init(&tattr);
  tattr.stacksize = CAM_DQ_THREAD_STACK_SIZE;
  param.sched_priority = CAM_DQ_THREAD_STACK_PRIO;
  pthread_attr_setschedparam(&tattr, &param);

  if (pthread_create(
        &dq_tid,
        &tattr,
        (pthread_startroutine_t)CameraClass::dqbuf_thread,
        (void *)this))
    {
      loop_dqbuf_en = false;
      CamImage *req = NULL;
      mq_send(frame_exchange_mq, (const char *)&req, sizeof(CamImage *), 0);
      pthread_join(frame_tid, NULL);
      mq_close(frame_exchange_mq);
      err = CAM_ERR_CANT_CREATE_THREAD;
    }
  else
    {
      pthread_setname_np(dq_tid, "cam_dq_thread");
    }

  return err;
}

void CameraClass::delete_dq_thread()
{
  if (loop_dqbuf_en)
    {
      loop_dqbuf_en = false;

      ioctl(video_fd, VIDIOC_CANCEL_DQBUF, V4L2_BUF_TYPE_VIDEO_CAPTURE);
      pthread_join(dq_tid, NULL);
      dq_tid = -1;

      CamImage *req = NULL;
      mq_send(frame_exchange_mq, (const char *)&req, sizeof(CamImage *), 0);
      pthread_join(frame_tid, NULL);
      frame_tid = -1;
      mq_close(frame_exchange_mq);
    }
}

// Public : Start to use the Camera.
CamErr CameraClass::begin(int buff_num, CAM_VIDEO_FPS fps, int video_width, int video_height,
                          CAM_IMAGE_PIX_FMT video_fmt, int jpgbufsize_divisor)
{
  CamErr ret = CAM_ERR_SUCCESS;

  if (buff_num < 0)
    {
      return CAM_ERR_INVALID_PARAM;
    }

  if ((video_fmt == CAM_IMAGE_PIX_FMT_JPG) && (jpgbufsize_divisor <= 0))
    {
      return CAM_ERR_INVALID_PARAM;
    }

  if (video_init_stat)
    {
      return CAM_ERR_NO_DEVICE;
    }

  if (video_fd >= 0)
    {
      return CAM_ERR_ALREADY_INITIALIZED;
    }

  video_fd = open(VIDEO_DEV_FILE_NAME, 0);
  if (video_fd < 0)
    {
      return CAM_ERR_NO_DEVICE;
    }

  imageproc_initialize();

  if (buff_num == 0)
    {
      return CAM_ERR_SUCCESS;
    }

  // Set Video Frame parameters.
  ret = set_frame_parameters(V4L2_BUF_TYPE_VIDEO_CAPTURE,
                             video_width,
                             video_height,
                             buff_num,
                             video_fmt);
  if (ret != CAM_ERR_SUCCESS)
    {
      goto label_err_no_memaligned;
    }

  // Set Video Frame Rate.
  ret = set_video_frame_rate(fps);
  if (ret != CAM_ERR_SUCCESS)
    {
      goto label_err_no_memaligned;
    }

  // Start Dequeue Buff thread.
  ret = create_dq_thread();
  if (ret != CAM_ERR_SUCCESS)
    {
      goto label_err_no_memaligned;
    }

  // Create Buffer
  ret = create_videobuff(video_width, video_height, buff_num, video_fmt,
                         jpgbufsize_divisor);
  if (ret != CAM_ERR_SUCCESS)
    {
      goto label_err_no_memaligned;
    }

  // Set Buffer into V4S
  ret = enqueue_video_buffs();
  if (ret != CAM_ERR_SUCCESS)
    {
      goto label_err_with_memaligned;
    }

  ret = set_video_frame_rate(fps);
  if (ret != CAM_ERR_SUCCESS)
    {
      goto label_err_with_memaligned;
    }

  video_pix_fmt = video_fmt;
  return ret; // Success begin.

  label_err_with_memaligned:
  delete_videobuff();

  label_err_no_memaligned:
  close(video_fd);
  video_fd = -1;

  return ret;
}

// Public : Start video streaming.
CamErr CameraClass::startStreaming(bool enable, camera_cb_t cb)
{
  camera_cb_t old_cb;
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  CamErr err = CAM_ERR_SUCCESS;
  unsigned long req = enable ? VIDIOC_STREAMON : VIDIOC_STREAMOFF;

  if (is_device_ready())
    {
      lock_video_cb();
      old_cb = video_cb;
      video_cb = cb;
      unlock_video_cb();

      if (ioctl(video_fd, req, (unsigned long)&type) < 0)
        {
          err =  convert_errno2camerr(errno);
          lock_video_cb();
          video_cb = old_cb;
          unlock_video_cb();
        }
    }
  else
    {
      err = CAM_ERR_NOT_INITIALIZED;
    }

  return err;
}

// Private : Set EXT_CTRLS of V4S.
CamErr CameraClass::set_ext_ctrls(uint16_t ctl_cls,
                                  uint16_t cid,
                                  int32_t  value)
{
  struct v4l2_ext_controls param = {0};
  struct v4l2_ext_control ctl_param = {0};

  if (is_device_ready())
    {
      ctl_param.id = cid;
      ctl_param.value = value;

      param.ctrl_class = ctl_cls;
      param.count = 1;
      param.controls = &ctl_param;

      if (ioctl(video_fd, VIDIOC_S_EXT_CTRLS, (unsigned long)&param) < 0)
        {
          return convert_errno2camerr(errno);
        }
      return CAM_ERR_SUCCESS;
    }
  else
    {
      return CAM_ERR_NOT_INITIALIZED;
    }
}

// Private : Get EXT_CTRLS of V4S.
int32_t CameraClass::get_ext_ctrls(uint16_t ctl_cls,
                                  uint16_t cid)
{
  struct v4l2_ext_controls param = {0};
  struct v4l2_ext_control ctl_param = {0};

  if (is_device_ready())
    {
      ctl_param.id = cid;

      param.ctrl_class = ctl_cls;
      param.count = 1;
      param.controls = &ctl_param;

      if (ioctl(video_fd, VIDIOC_G_EXT_CTRLS, (unsigned long)&param) < 0)
        {
          return convert_errno2camerr(errno);
        }

      return ctl_param.value;
    }
  else
    {
      return CAM_ERR_NOT_INITIALIZED;
    }
}

// Public : Turn on/off Auto White Balance.
CamErr CameraClass::setAutoWhiteBalance(bool enable)
{
  return set_ext_ctrls(V4L2_CTRL_CLASS_USER,
                       V4L2_CID_AUTO_WHITE_BALANCE,
                       enable ? 1 : 0 );
}

// Public : Turn on/off Auto Exposure.
CamErr CameraClass::setAutoExposure(bool enable)
{
  return set_ext_ctrls(V4L2_CTRL_CLASS_CAMERA,
                       V4L2_CID_EXPOSURE_AUTO,
                       enable ? V4L2_EXPOSURE_AUTO : V4L2_EXPOSURE_MANUAL);
}

// Public : Set exposure time in ms.
CamErr CameraClass::setAbsoluteExposure(int32_t exposure_time)
{
  return set_ext_ctrls(V4L2_CTRL_CLASS_CAMERA,
                       V4L2_CID_EXPOSURE_ABSOLUTE,
                       exposure_time);
}

// Public : Get exposure time in 100usec.
int32_t CameraClass::getAbsoluteExposure(void)
{
  return get_ext_ctrls(V4L2_CTRL_CLASS_CAMERA,
                       V4L2_CID_EXPOSURE_ABSOLUTE);
}

// Public : Turn on/off Auto ISO Sensitivity.
CamErr CameraClass::setAutoISOSensitivity(bool enable)
{
  return set_ext_ctrls(V4L2_CTRL_CLASS_CAMERA,
                       V4L2_CID_ISO_SENSITIVITY_AUTO,
                       enable ? V4L2_ISO_SENSITIVITY_AUTO
                        : V4L2_ISO_SENSITIVITY_MANUAL );
}

/* Will obsolete after v1.2.0 */
CamErr CameraClass::setAutoISOSensitive(bool enable)
{
  return setAutoISOSensitivity(enable);
}

// Public : Set ISO Sensitivity value in manual.
CamErr CameraClass::setISOSensitivity(int iso_sense)
{
  return set_ext_ctrls(V4L2_CTRL_CLASS_CAMERA,
                       V4L2_CID_ISO_SENSITIVITY,
                       (uint32_t)iso_sense );
}

// Public : Get ISO Sensitivity value.
int CameraClass::getISOSensitivity(void)
{
  return get_ext_ctrls(V4L2_CTRL_CLASS_CAMERA,
                       V4L2_CID_ISO_SENSITIVITY);
}

// Public : Auto White Balance Mode.
CamErr CameraClass::setAutoWhiteBalanceMode(CAM_WHITE_BALANCE wb)
{
  return set_ext_ctrls(V4L2_CTRL_CLASS_CAMERA,
                       V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE,
                       (uint32_t)wb);
}
#if 0 /* To Be Supported */
// Public : Scene Mode.
CamErr CameraClass::setSceneMode(CAM_SCENE_MODE mode)
{
  return set_ext_ctrls(V4L2_CTRL_CLASS_CAMERA,
                       V4L2_CID_SCENE_MODE,
                       (uint32_t)mode );
}
#endif
// Public : Color Effect
CamErr CameraClass::setColorEffect(CAM_COLOR_FX effect)
{
  return set_ext_ctrls(V4L2_CTRL_CLASS_USER,
                       V4L2_CID_COLORFX,
                       (uint32_t)effect );
}

// Public : HDR
CamErr CameraClass::setHDR(CAM_HDR_MODE mode)
{
  return set_ext_ctrls(V4L2_CTRL_CLASS_CAMERA,
                       V4L2_CID_WIDE_DYNAMIC_RANGE,
                       mode);
}

CAM_HDR_MODE CameraClass::getHDR(void)
{
  int ret;

  ret = get_ext_ctrls(V4L2_CTRL_CLASS_CAMERA, V4L2_CID_WIDE_DYNAMIC_RANGE);
  ASSERT(ret >= 0);
  return (CAM_HDR_MODE)ret;
}

// Public : JPEG quality
CamErr CameraClass::setJPEGQuality(int quality)
{
  return set_ext_ctrls(V4L2_CTRL_CLASS_JPEG,
                       V4L2_CID_JPEG_COMPRESSION_QUALITY,
                       quality);
}

int CameraClass::getJPEGQuality(void)
{
  return  get_ext_ctrls(V4L2_CTRL_CLASS_JPEG,
                        V4L2_CID_JPEG_COMPRESSION_QUALITY);
}

// Public : frame interval
int CameraClass::getFrameInterval(void)
{
  struct v4l2_streamparm param = {0};
  struct v4l2_fract *interval = &param.parm.capture.timeperframe;

  if (is_device_ready())
    {
      param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

      if (ioctl(video_fd, VIDIOC_G_PARM, (unsigned long)&param) < 0)
        {
          return convert_errno2camerr(errno);
        }

      /* Convert fraction to value with 100usec unit. */

      return interval->numerator * 10000 / interval->denominator;
    }
  else
    {
      return CAM_ERR_NOT_INITIALIZED;
    }
}

// Public : Still Picture Format.
CamErr CameraClass::setStillPictureImageFormat(int img_width, int img_height, CAM_IMAGE_PIX_FMT img_fmt,
                                               int jpgbufsize_divisor)
{
  CamErr err = CAM_ERR_SUCCESS;

  if ((img_fmt == CAM_IMAGE_PIX_FMT_JPG) && (jpgbufsize_divisor <= 0))
    {
      return CAM_ERR_INVALID_PARAM;
    }

  if (is_device_ready())
    {
      err = set_frame_parameters(V4L2_BUF_TYPE_STILL_CAPTURE,
                                 img_width, img_height, 1, img_fmt);
      if (err == CAM_ERR_SUCCESS)
        {
          err = create_stillbuff(img_width, img_height, img_fmt, jpgbufsize_divisor);
          if (err == CAM_ERR_SUCCESS)
            {
              enqueue_video_buff(still_img);
            }
        }
    }
  else
    {
      err = CAM_ERR_NOT_INITIALIZED;
    }

  still_pix_fmt = img_fmt;
  return err;
}

// Public : Take a Picture.
CamImage CameraClass::takePicture( )
{
  struct v4l2_buffer buf;
  long unsigned int take_num = 0; /* Currently, not support positive value. */

  if (is_device_ready())
    {
      if(still_img && still_img->img_buff->is_queued())
        {
          if (ioctl(video_fd, VIDIOC_TAKEPICT_START, take_num) == 0)
            {
              if (ioctl_dequeue_stream_buf(&buf, V4L2_BUF_TYPE_STILL_CAPTURE) == 0)
                {
                  still_img->img_buff->queued(false);
                  if (ioctl(video_fd, VIDIOC_TAKEPICT_STOP, false) == 0)
                    {
                      if ((buf.flags & V4L2_BUF_FLAG_ERROR) == 0)
                        {
                          still_img->setActualSize((size_t)buf.bytesused);
                        }
                      else
                        {
                          still_img->setActualSize((size_t)0);
                        }

                      still_img->setPixFormat(still_pix_fmt);
                      return *still_img;
                    }
                }
            }
        }
    }

  return CamImage();  // Return empty CamImage because of any error occured.
}

// Public : Get camera device type.
CAM_DEVICE_TYPE CameraClass::getDeviceType()
{
  CAM_DEVICE_TYPE ret;
  struct v4l2_capability param = {0};

  ret = CAM_DEVICE_TYPE_UNKNOWN;

  if (is_device_ready())
    {
      if (ioctl(video_fd, VIDIOC_QUERYCAP, (unsigned long)&param) == 0)
        {
          if (strncmp((char *)param.driver, "ISX012", sizeof(param.driver)) == 0)
            {
              ret = CAM_DEVICE_TYPE_ISX012;
            }
          else if (strncmp((char *)param.driver, "ISX019", sizeof(param.driver)) == 0)
            {
              ret = CAM_DEVICE_TYPE_ISX019;
            }
        }
    }

  return ret;
}

// Public : Finish to use the Camera.
void CameraClass::end()
{
  if (is_device_ready())
    {
      delete_dq_thread();

      close(video_fd);
      video_fd = -1;

      delete_videobuff();
      DELETE_CAMIMAGE(still_img);

      imageproc_finalize();
    }
}

int CameraClass::ioctl_dequeue_stream_buf(struct v4l2_buffer *buf,
                                          uint16_t type)
{
  memset(buf, 0, sizeof(v4l2_buffer_t));
  buf->type = type;
  buf->memory = V4L2_MEMORY_USERPTR;
  return ioctl(video_fd, VIDIOC_DQBUF, (unsigned long)buf);
}

CamImage *CameraClass::search_vimg(int index)
{
  CamImage *img = NULL;

  for (int i = 0; i < video_buf_num; i++)
    {
      if (video_imgs[i]->isIdx(index))
        {
          img = video_imgs[i];
          break;
        }
    }

  return img;
}

// Private Static : dqbuf buffer handling thread.
void CameraClass::dqbuf_thread(void *arg)
{
  struct v4l2_buffer buf;
  CameraClass *cam = (CameraClass *)arg;

  while (cam->loop_dqbuf_en)
    {
      if (cam->ioctl_dequeue_stream_buf(&buf, V4L2_BUF_TYPE_VIDEO_CAPTURE)
           == 0)
        {
          CamImage *img = cam->search_vimg(buf.index);
          if (img != NULL)
            {
              img->img_buff->queued(false);

              if ((buf.flags & V4L2_BUF_FLAG_ERROR) == 0)
                {
                  img->setActualSize((size_t)buf.bytesused);
                }
              else
                {
                  img->setActualSize((size_t)0);
                }

              if(mq_send(cam->frame_exchange_mq, (const char *)&img, sizeof(CamImage *), 0)
                  < 0)
                {
                  // in error case, the buf returns camera queue.
                  cam->enqueue_video_buff(img);
                }
            }
        }
    }
  pthread_exit(0);
}

// Private Static : dqbuf buffer handling thread.
void CameraClass::frame_handle_thread(void *arg)
{
  CameraClass *cam = (CameraClass *)arg;
  CamImage *img = NULL;

  while (cam->loop_dqbuf_en)
    {
      if(mq_receive(cam->frame_exchange_mq, (char *)&img, sizeof(CamImage *), 0)
           >= 0)
        {
          if(img)
            {
              cam->lock_video_cb();
              if (cam->video_cb != NULL)
                {
                  img->setPixFormat(cam->video_pix_fmt);
                  cam->video_cb(*img);
                }
              else
                {
                  // No one handles this. Back to queue then.
                  cam->enqueue_video_buff(img);
                }
              cam->unlock_video_cb();
            }
        }
    }
  pthread_exit(0);
}


// Private Static :
void CameraClass::release_buf(ImgBuff *buf)
{
  int idx = buf->idx;
  if (still_img && still_img->isIdx(idx))
    {
      enqueue_video_buff(still_img);
      return;
    }

  for (int i=0; i < video_buf_num; i++)
    {
      if (video_imgs[i]->isIdx(idx))
        {
          enqueue_video_buff(video_imgs[i]);
          break;
        }
    }
}


/** Global instance */
CameraClass theCamera = CameraClass::getInstance();

