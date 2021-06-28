/*
 *  MediaPlayer.cpp - SPI implement file for the Spresense SDK
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <arch/board/board.h>

#include "MediaPlayer.h"
#include "MemoryUtil.h"


#include <File.h>

void  input_device_callback(uint32_t size)
{
    /* do nothing */
}

extern "C" {

static void attentionCallback(const ErrorAttentionParam *attparam)
{
  print_err("Attention!! Level 0x%x Code 0x%lx\n", attparam->error_code, attparam->error_att_sub_code);
}

}

/****************************************************************************
 * Public API on MediaPlayer Class
 ****************************************************************************/

err_t MediaPlayer::begin(void)
{
  return MEDIAPLAYER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::create(PlayerId id)
{
  return create(id, NULL);
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::create(PlayerId id, AudioAttentionCb attcb)
{
  AsCreatePlayerParams_t player_create_param;

  player_create_param.msgq_id.player = (id == Player0) ? MSGQ_AUD_PLY : MSGQ_AUD_SUB_PLY;
  player_create_param.msgq_id.mng    = MSGQ_AUD_MGR;
  player_create_param.msgq_id.mixer  = MSGQ_AUD_OUTPUT_MIX;
  player_create_param.msgq_id.dsp    = (id == Player0) ? MSGQ_AUD_DSP : MSGQ_AUD_SUB_DSP;
  player_create_param.pool_id.es     = (id == Player0) ? S0_DEC_ES_MAIN_BUF_POOL : S0_DEC_ES_SUB_BUF_POOL;
  player_create_param.pool_id.pcm    = (id == Player0) ? S0_REND_PCM_BUF_POOL : S0_REND_PCM_SUB_BUF_POOL;
  player_create_param.pool_id.dsp    = S0_DEC_APU_CMD_POOL;
  player_create_param.pool_id.src_work = (id == Player0) ? S0_SRC_WORK_MAIN_BUF_POOL : S0_SRC_WORK_SUB_BUF_POOL;

  bool result;

  if (id == Player0)
    {
      result = AS_CreatePlayerMulti(AS_PLAYER_ID_0, &player_create_param, (attcb) ? attcb : attentionCallback);
    }
  else
    {
      result = AS_CreatePlayerMulti(AS_PLAYER_ID_1, &player_create_param, (attcb) ? attcb : attentionCallback);
    }

  if (!result)
    {
      print_err("Error: AS_CratePlayer() failure. system memory insufficient!\n");
      return MEDIAPLAYER_ECODE_COMMAND_ERROR;
    }

  return MEDIAPLAYER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::activate(PlayerId id, MediaPlayerCallback mpcb)
{
  return MediaPlayer::activate(id, AS_SETPLAYER_OUTPUTDEVICE_SPHP, mpcb);
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::activate(PlayerId id, MediaPlayerCallback mpcb, uint32_t player_bufsize)
{
  return MediaPlayer::activate(id, AS_SETPLAYER_OUTPUTDEVICE_SPHP, mpcb, player_bufsize);
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::activate(PlayerId id, uint8_t output_device, MediaPlayerCallback mpcb)
{
  return MediaPlayer::activate(id, output_device, mpcb, MEDIAPLAYER_BUF_SIZE);
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::activate(PlayerId id, uint8_t output_device, MediaPlayerCallback mpcb, uint32_t player_bufsize)
{
  /* Buffer size check */

  if (!player_bufsize)
    {
      print_err("Invalid buffer size.\n");
      return MEDIAPLAYER_ECODE_BUFFERSIZE_ERROR;
    }

  /* Alloc buffer */

  CMN_SimpleFifoHandle handle = { 0 };

  uint32_t *p_buffer = static_cast<uint32_t *>(malloc(player_bufsize));

  if (!p_buffer)
    {
      print_err("Buffer allocate error.\n");
      return MEDIAPLAYER_ECODE_BUFFERALLOC_ERROR;
    }

  if (CMN_SimpleFifoInitialize(&handle,
                               p_buffer,
                               player_bufsize,
                               NULL) != 0)
    {
      print_err("Fail to initialize simple FIFO.\n");
      free(p_buffer);
      return MEDIAPLAYER_ECODE_SIMPLEFIFO_ERROR;
    }

  CMN_SimpleFifoClear(&handle);

  if (id == Player0)
    {
      m_player0_simple_fifo_handle = handle;
      m_player0_simple_fifo_buf    = p_buffer;
      m_player0_input_device_handler.simple_fifo_handler = &m_player0_simple_fifo_handle; 
      m_player0_input_device_handler.callback_function   = input_device_callback; 
    }
  else
    {
      m_player1_simple_fifo_handle = handle;
      m_player1_simple_fifo_buf    = p_buffer;
      m_player1_input_device_handler.simple_fifo_handler = &m_player1_simple_fifo_handle; 
      m_player1_input_device_handler.callback_function   = input_device_callback; 
    }

  /* Activate */

  AsPlayerInputDeviceHdlrForRAM *p_input_dev_handler =
    (id == Player0) ?
      &m_player0_input_device_handler : &m_player1_input_device_handler;

  AsActivatePlayer player_act;

  player_act.param.input_device  = AS_SETPLAYER_INPUTDEVICE_RAM;
  player_act.param.ram_handler   = p_input_dev_handler;
  player_act.param.output_device = output_device/*AS_SETPLAYER_OUTPUTDEVICE_SPHP*/;
  player_act.cb                  = mpcb;

  if (id == Player0)
    {
      AS_ActivatePlayer(AS_PLAYER_ID_0, &player_act);
    }
  else
    {
      AS_ActivatePlayer(AS_PLAYER_ID_1, &player_act);
    }

  return MEDIAPLAYER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::init(PlayerId id,
                        uint8_t codec_type,
                        uint32_t sampling_rate,
                        uint8_t channel_number)
{
  return init(id, codec_type, "/mnt/sd0/BIN", sampling_rate, AS_BITLENGTH_16, channel_number);
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::init(PlayerId id,
                        uint8_t codec_type,
                        uint32_t sampling_rate,
                        uint8_t bit_length,
                        uint8_t channel_number)
{
  return init(id, codec_type, "/mnt/sd0/BIN", sampling_rate, bit_length, channel_number);
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::init(PlayerId id,
                        uint8_t codec_type,
                        const char *codec_path,
                        uint32_t sampling_rate,
                        uint8_t channel_number)
{
  return init(id, codec_type, codec_path, sampling_rate, AS_BITLENGTH_16, channel_number);
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::init(PlayerId id,
                        uint8_t codec_type,
                        const char *codec_path,
                        uint32_t sampling_rate,
                        uint8_t bit_length,
                        uint8_t channel_number)
{
  if (!check_decode_dsp(codec_type, codec_path))
    {
      return MEDIAPLAYER_ECODE_FILEACCESS_ERROR;
    }

  AsInitPlayerParam player_init;

  player_init.codec_type     = codec_type/*AS_CODECTYPE_MP3*/;
  player_init.bit_length     = bit_length;
  player_init.channel_number = channel_number/*AS_CHANNEL_STEREO*/;
  player_init.sampling_rate  = sampling_rate/*AS_SAMPLINGRATE_48000*/;
  snprintf(player_init.dsp_path, AS_AUDIO_DSP_PATH_LEN, "%s", codec_path);

  if (id == Player0)
    {
      AS_InitPlayer(AS_PLAYER_ID_0, &player_init);
    }
  else
    {
      AS_InitPlayer(AS_PLAYER_ID_1, &player_init);
    }

  return MEDIAPLAYER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::start(PlayerId id, DecodeDoneCallback dccb)
{
  board_external_amp_mute_control(false);

  AsPlayPlayerParam player_play;

  player_play.pcm_path = AsPcmDataReply;
  player_play.pcm_dest.callback = dccb;

  if (id == Player0)
    {
      AS_PlayPlayer(AS_PLAYER_ID_0, &player_play);
    }
  else
    {
      AS_PlayPlayer(AS_PLAYER_ID_1, &player_play);
    }

  return MEDIAPLAYER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::stop(PlayerId id)
{
  return stop(id, AS_STOPPLAYER_ESEND);
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::stop(PlayerId id, uint8_t mode)
{
  AsStopPlayerParam player_stop;

  player_stop.stop_mode = mode;

  if (id == Player0)
    {
      AS_StopPlayer(AS_PLAYER_ID_0, &player_stop);
    }
  else
    {
      AS_StopPlayer(AS_PLAYER_ID_1, &player_stop);
    }

  return MEDIAPLAYER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::reqNextProcess(PlayerId id, AsRequestNextType type)
{
  AsRequestNextParam next;

  next.type = type;

  if (id == Player0)
    {
      AS_RequestNextPlayerProcess(AS_PLAYER_ID_0, &next);
    }
  else
    {
      AS_RequestNextPlayerProcess(AS_PLAYER_ID_1, &next);
    }

  return MEDIAPLAYER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::deactivate(PlayerId id)
{
  AsDeactivatePlayer player_deact;

  if (id == Player0)
    {
      AS_DeactivatePlayer(AS_PLAYER_ID_0, &player_deact);
    }
  else
    {
      AS_DeactivatePlayer(AS_PLAYER_ID_1, &player_deact);
    }

  /* Free buffer */
  
  void *p_buffer = NULL;

  if (id == Player0)
    {
      p_buffer = m_player0_simple_fifo_buf;
      m_player0_simple_fifo_buf = NULL;
    }
  else
    {
      p_buffer = m_player1_simple_fifo_buf;
      m_player1_simple_fifo_buf = NULL;
    }

  free(p_buffer);

  return MEDIAPLAYER_ECODE_OK;
}


#define WRITE_FRAME_NUM 5
/*--------------------------------------------------------------------------*/
err_t MediaPlayer::writeFrames(PlayerId id, File& myFile)
{
  int ret = MEDIAPLAYER_ECODE_OK;

  uint32_t *p_fifo = (id == Player0)
    ? m_player0_simple_fifo_buf : m_player1_simple_fifo_buf;

  if (!p_fifo)
    {
      print_err("FIFO area is not allocated.\n");
      return MEDIAPLAYER_ECODE_SIMPLEFIFO_ERROR;
    }

  char *buf = (id == Player0) ? m_es_player0_buf : m_es_player1_buf;

  CMN_SimpleFifoHandle *handle =
    (id == Player0) ?
      &m_player0_simple_fifo_handle : &m_player1_simple_fifo_handle;

  for (int i = 0; i < WRITE_FRAME_NUM; i++)
    {
      ret = write_fifo(myFile, buf, handle);
      if (ret != MEDIAPLAYER_ECODE_OK) break;
    }

  return ret;
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::write_fifo(File& myFile, char *p_es_buf, CMN_SimpleFifoHandle *handle)
{

  int vacant_size = CMN_SimpleFifoGetVacantSize(handle);
  if (vacant_size < MEDIAPLAYER_BUF_FRAME_SIZE)
    {
      return MEDIAPLAYER_ECODE_OK;
    }

  int ret = -1;

  if (myFile.available())
    {
      ret = myFile.read(p_es_buf, MEDIAPLAYER_BUF_FRAME_SIZE);
    }
  else
    {
      ret = 0;
    }

  if (ret < 0)
    {
      print_err("Fail to read file. errno:%d\n", errno);
      return MEDIAPLAYER_ECODE_FILEACCESS_ERROR;
    }

  if(ret == 0)
    {
      return MEDIAPLAYER_ECODE_FILEEND;
    }

  if (CMN_SimpleFifoOffer(handle, (const void*)(p_es_buf), ret) == 0)
    {
      print_err("Simple FIFO is full!\n");
      return MEDIAPLAYER_ECODE_SIMPLEFIFO_ERROR;
    }

  return MEDIAPLAYER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::writeFrames(PlayerId id, uint8_t *data, uint32_t size)
{
  int ret = MEDIAPLAYER_ECODE_OK;

  uint32_t *p_fifo = (id == Player0)
    ? m_player0_simple_fifo_buf : m_player1_simple_fifo_buf;

  if (!p_fifo)
    {
      print_err("FIFO area is not allocated.\n");
      return MEDIAPLAYER_ECODE_SIMPLEFIFO_ERROR;
    }

  CMN_SimpleFifoHandle *handle =
    (id == Player0) ?
      &m_player0_simple_fifo_handle : &m_player1_simple_fifo_handle;

  ret = write_fifo(data, size, handle);

  return ret;
}

/*--------------------------------------------------------------------------*/
err_t MediaPlayer::write_fifo(uint8_t *data, uint32_t size, CMN_SimpleFifoHandle *handle)
{
  uint32_t vacant_size = CMN_SimpleFifoGetVacantSize(handle);

  if (vacant_size < size)
    {
      return MEDIAPLAYER_ECODE_SIMPLEFIFO_ERROR;
    }

  if (!data || !size)
    {
      return MEDIAPLAYER_ECODE_COMMAND_ERROR;
    }

  if (CMN_SimpleFifoOffer(handle, (const void*)data, size) == 0)
    {
      print_err("Simple FIFO is full!\n");
      return MEDIAPLAYER_ECODE_SIMPLEFIFO_ERROR;
    }

  return MEDIAPLAYER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
bool MediaPlayer::check_decode_dsp(uint8_t codec_type, const char *path)
{
  char fullpath[32] = { 0 };
  struct stat buf;
  int retry;
  int ret = 0;

  switch (codec_type)
    {
      case AS_CODECTYPE_MP3:
        snprintf(fullpath, sizeof(fullpath), "%s/MP3DEC", path);
        break;

      case AS_CODECTYPE_AAC:
      case AS_CODECTYPE_MEDIA:
        snprintf(fullpath, sizeof(fullpath), "%s/AACDEC", path);
        break;

      case AS_CODECTYPE_WAV:
      case AS_CODECTYPE_LPCM:
        snprintf(fullpath, sizeof(fullpath), "%s/WAVDEC", path);
        break;

      case AS_CODECTYPE_OPUS:
        snprintf(fullpath, sizeof(fullpath), "%s/OPUSDEC", path);
        break;

      default:
        print_err("Codec type %d is invalid value.\n", codec_type);
        return false;
    }

  if (0 == strncmp("/mnt/sd0", path, 8))
    {
      /* In case that SD card isn't inserted, it times out at max 2 sec */
      for (retry = 0; retry < 20; retry++) {
        ret = stat("/mnt/sd0", &buf);
        if (ret == 0)
          {
            break;
          }
        usleep(100 * 1000); // 100 msec
      }
      if (ret)
        {
          print_err("SD card is not present.\n");
          return false;
        }
    }

  FILE *fp = fopen(fullpath, "r");
  if (fp == NULL)
    {
      print_err("DSP file %s cannot open.\n", fullpath);
      return false;
    }

  fclose(fp);

  return true;
}


