/*
 *  Camera.cpp - Camera implement file for the Spresense SDK
 *  Copyright 2018 Sony Semiconductor Solutions Corporation
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

//***************************************************************************
// Included Files
//***************************************************************************
#include <arch/board/board.h>

#include <sdk/config.h>

#include <sys/ioctl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <asmp/mpshm.h>

#include "Camera.h"

//***************************************************************************
// Definitions
//***************************************************************************

#define KEY_SHM        (0)
#define IMAGE_JPG_SIZE (896*1024)

struct v_buffer {
  uint32_t *start;
  uint32_t length;
};

struct v_buffer *buffers;
static int       v_fd;

/****************************************************************************
 * Common API on Camera Class
 ****************************************************************************/
err_t CameraClass::begin(void)
{
  err_t ret=0;

  ret = board_isx012_initialize("/dev/video", IMAGER_I2C);
  if (ret != 0)
    {
      printf("ERROR: Failed to init video. %d\n", errno);
      return -EPERM;
    }

  v_fd = open("/dev/video0", O_CREAT);
  if (v_fd < 0)
    {
      printf("failed to open imager driver : %d:%d\n", v_fd, errno);
      return 1;
    }

  return ret;
}

/*--------------------------------------------------------------------------*/
err_t CameraClass::end(void)
{
  err_t ret=0;

  ret = board_isx012_uninitialize();
  if (ret != 0)
    {
      printf("ERROR: Failed to init video. %d\n", errno);
      return -EPERM;
    }

  close(v_fd);
  free(buffers->start);

  return ret;
}

/*--------------------------------------------------------------------------*/
err_t CameraClass::initialize()
{
  err_t ret=0;

  uint32_t mode = V4L2_PIX_FMT_JPEG;
  enum   v4l2_buf_type       type;
  struct v4l2_format         fmt;
  struct v4l2_requestbuffers req;
  struct v4l2_buffer         buf;

  /*-init_device------------------------------------------------*/
  memset(&fmt, 0, sizeof(v4l2_format_t));
  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width       = VIDEO_HSIZE_FULLHD;
  fmt.fmt.pix.height      = VIDEO_VSIZE_FULLHD;
  fmt.fmt.pix.pixelformat = mode;
  fmt.fmt.pix.field       = V4L2_FIELD_ANY;
  ret = ioctl(v_fd, VIDIOC_S_FMT, (unsigned long)&fmt);
  if (ret)
    {
      printf("Fail set format %d\n", errno);
      return -1;
    }

  /*-init_device/init_userp-------------------------------------*/
  memset(&req, 0, sizeof(v4l2_requestbuffers_t));
  req.count  = 1;
  req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_USERPTR;

  ret = ioctl(v_fd, VIDIOC_REQBUFS, (unsigned long)&req);
  if (ret)
    {
      printf("Does not support user pointer i/o %d\n", errno);
      return -1;
    }

  /* Get MP shared memory for JPEG capture */

  ret = mpshm_init(&shm, KEY_SHM, IMAGE_JPG_SIZE);
  if (ret < 0)
    {
      printf("mpshm_init() failure. %d\n", ret);
      return ret;
    }

  uint8_t *vbuffer = (uint8_t *)mpshm_attach(&shm, 0);
  if (!vbuffer)
    {
      printf("mpshm_attach() failure.\n");
      return ret;
    }

  buffer = (uint8_t *)mpshm_virt2phys(&shm, (void *)vbuffer);

  buffers->length = IMAGE_JPG_SIZE;
  buffers->start  = (uint32_t *)buffer;

  /*-start_capturing--------------------------------------------*/
  memset(&buf, 0, sizeof(v4l2_buffer_t));
  buf.type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory    = V4L2_MEMORY_USERPTR;
  buf.index     = 0;
  buf.m.userptr = (unsigned long)buffers->start;
  buf.length    = buffers->length;

  ret = ioctl(v_fd, VIDIOC_QBUF, (unsigned long)&buf);
  if (ret)
    {
      printf("Fail QBUF %d\n", errno);
      return -1;
    }

  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ret = ioctl(v_fd, VIDIOC_STREAMON, (unsigned long)&type);
  if (ret)
    {
      printf("Fail STREAMON %d\n", errno);
      return -1;
    }

  return ret;
}

/*--------------------------------------------------------------------------*/
err_t CameraClass::read(File& myFile)
{
  err_t ret=0;
  struct v4l2_buffer         buf;

  memset(&buf, 0, sizeof(v4l2_buffer_t));
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_USERPTR;

  ret = ioctl(v_fd, VIDIOC_DQBUF, (unsigned long)&buf);
  if (ret)
    {
      printf("Fail DQBUF %d\n", errno);
      return -1;
    }

  myFile.write((uint8_t *)buf.m.userptr, buf.bytesused);
  myFile.close();

  ret = ioctl(v_fd, VIDIOC_QBUF, (unsigned long)&buf);
  if (ret)
    {
      printf("Fail QBUF %d\n", errno);
      return -1;
    }

  return ret;

}
