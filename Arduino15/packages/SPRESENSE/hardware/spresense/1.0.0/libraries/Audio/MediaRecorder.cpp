/*
 *  MediaRecorder.cpp - SPI implement file for the Spresense SDK
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
#include <arch/chip/cxd56_audio.h>

#include "MediaRecorder.h"

#include "memutil/msgq_id.h"
#include "memutil/mem_layout.h"
#include "memutil/memory_layout.h"

/*--------------------------------------------------------------------------*/
void  output_device_callback(uint32_t size)
{
    /* do nothing */
}

/****************************************************************************
 * Public API on MediaRecorder Class
 ****************************************************************************/

err_t MediaRecorder::begin(void)
{
  /* Allocate ES buffer */

  m_recorder_simple_fifo_buf =
    (uint32_t*)(0xfffffffc & ((uint32_t)(malloc(MEDIARECORDER_BUF_SIZE + 3)) + 3));

  bool result;

  /* Create MediaRecorder feature. */

  AsCreateRecorderParam_t recorder_create_param;

  recorder_create_param.msgq_id.recorder = MSGQ_AUD_RECORDER; 
  recorder_create_param.msgq_id.mng      = MSGQ_AUD_MGR;
  recorder_create_param.msgq_id.dsp      = MSGQ_AUD_DSP;
  recorder_create_param.pool_id.input    = MIC_IN_BUF_POOL;
  recorder_create_param.pool_id.output   = OUTPUT_BUF_POOL;
  recorder_create_param.pool_id.dsp      = ENC_APU_CMD_POOL;

  result = AS_CreateMediaRecorder(&recorder_create_param);
  if (!result)
    {
      print_err("Error: AS_CreateMediaRecorder() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  /* Create Capture feature. */

  AsCreateCaptureParam_t capture_create_param;

  capture_create_param.msgq_id.dev0_req  = MSGQ_AUD_CAP;
  capture_create_param.msgq_id.dev0_sync = MSGQ_AUD_CAP_SYNC;
  capture_create_param.msgq_id.dev1_req  = 0xFF;
  capture_create_param.msgq_id.dev1_sync = 0xFF;

  result = AS_CreateCapture(&capture_create_param);
  if (!result)
    {
      print_err("Error: As_CreateCapture() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  return MEDIARECORDER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::end(void)
{
  /* Free ES buffer */

  free((void*)m_recorder_simple_fifo_buf);

  bool result;

  /* Delete MediaRecorder */

  result = AS_DeleteMediaRecorder();
  if (!result)
    {
      print_err("Error: AS_DeleteMediaRecorder() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  /* Delete Capture */

  result = AS_DeleteCapture();
  if (!result)
    {
      print_err("Error: AS_DeleteCapture() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  return MEDIARECORDER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::activate(AsSetRecorderStsInputDevice input_device, MediaRecorderCallback mrcb)
{
  if (CMN_SimpleFifoInitialize(&m_recorder_simple_fifo_handle,
                               m_recorder_simple_fifo_buf,
                               MEDIARECORDER_BUF_SIZE,
                               NULL) != 0)
    {
      print_err("Fail to initialize simple FIFO.\n");
      return MEDIARECORDER_ECODE_BUFFER_INIT_ERROR;
    }

  CMN_SimpleFifoClear(&m_recorder_simple_fifo_handle);

  /* Activate MediaRecorder */

  bool result;

  AsActivateRecorder recorder_act;

  m_output_device_handler.simple_fifo_handler = (void*)(&m_recorder_simple_fifo_handle);
  m_output_device_handler.callback_function   = output_device_callback;

  recorder_act.param.input_device          = input_device;
  recorder_act.param.output_device         = AS_SETRECDR_STS_OUTPUTDEVICE_RAM;
  recorder_act.param.input_device_handler  = 0x00;
  recorder_act.param.output_device_handler = &m_output_device_handler;
  recorder_act.cb                          = mrcb;

  result = AS_ActivateMediaRecorder(&recorder_act);
  if (!result)
    {
      print_err("Error: AS_ActivateMediaRecorder() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  /* Activate Baseband */

  result = activateBaseband();
  if (!result)
    {
      print_err("Error: Baseband activation() failure!\n");
      return MEDIARECORDER_ECODE_BASEBAND_ERROR;
    }

  return MEDIARECORDER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::init(uint8_t codec_type,
                          uint8_t channel_number)
{
  return init(codec_type,
              channel_number,
              AS_SAMPLINGRATE_48000,
              AS_BITLENGTH_16,
              AS_SAMPLINGRATE_96000,
              "/mnt/sd0/BIN");
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::init(uint8_t codec_type,
                          uint8_t channel_number,
                          uint32_t sampling_rate,
                          uint8_t bit_length,
                          uint32_t bit_rate)
{
  return init(codec_type,
              channel_number,
              sampling_rate,
              bit_length,
              bit_rate,
              "/mnt/sd0/bin");
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::init(uint8_t codec_type,
                          uint8_t channel_number,
                          uint32_t sampling_rate,
                          uint8_t bit_length,
                          uint32_t bit_rate,
                          const char *codec_path)
{
  if (!check_encode_dsp(codec_type, codec_path, sampling_rate))
    {
      return MEDIARECORDER_ECODE_DSP_ACCESS_ERROR;
    }

  /* Init MediaRecorder */

  AsInitRecorderParam init_param;
  bool result;

  init_param.sampling_rate  = sampling_rate;
  init_param.channel_number = channel_number;
  init_param.bit_length     = bit_length;
  init_param.bitrate        = bit_rate;
  snprintf(init_param.dsp_path, AS_AUDIO_DSP_PATH_LEN, "%s", codec_path);

  switch (codec_type)
    {
      case AS_CODECTYPE_WAV:
        init_wav(&init_param);
        break;

      case AS_CODECTYPE_MP3:
        init_mp3(&init_param);
        break;

      case AS_CODECTYPE_OPUS:
        init_opus(&init_param);
        break;

      case AS_CODECTYPE_LPCM:
        init_pcm(&init_param);
        break;

      default:
        break;
    }

  result = AS_InitMediaRecorder(&init_param);
  if (!result)
    {
      print_err("Error: AS_InitMediaRecorder() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  return MEDIARECORDER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
void MediaRecorder::init_wav(AsInitRecorderParam *param)
{
  param->codec_type = AS_CODECTYPE_LPCM;

  /* Create WAV header information */

  memcpy(m_wav_format.riff, CHUNKID_RIFF, strlen(CHUNKID_RIFF));
  memcpy(m_wav_format.wave, FORMAT_WAVE, strlen(FORMAT_WAVE));
  memcpy(m_wav_format.fmt, SUBCHUNKID_FMT, strlen(SUBCHUNKID_FMT));
  m_wav_format.fmt_size = FMT_SIZE;
  m_wav_format.format   = AUDIO_FORMAT_PCM;
  m_wav_format.channel  = param->channel_number;
  m_wav_format.rate     = param->sampling_rate;
  m_wav_format.avgbyte  = param->sampling_rate * param->channel_number * (param->bit_length / 8);
  m_wav_format.block    = param->channel_number * (param->bit_length / 8);
  m_wav_format.bit      = param->bit_length;
  memcpy(m_wav_format.data, SUBCHUNKID_DATA, strlen(SUBCHUNKID_DATA));
}

/*--------------------------------------------------------------------------*/
void MediaRecorder::init_mp3(AsInitRecorderParam *param)
{
  param->codec_type = AS_CODECTYPE_MP3;
}

/*--------------------------------------------------------------------------*/
void MediaRecorder::init_opus(AsInitRecorderParam *param)
{
  param->codec_type               = AS_CODECTYPE_OPUS;
  param->computational_complexity = AS_INITREC_COMPLEXITY_0;
}

/*--------------------------------------------------------------------------*/
void MediaRecorder::init_pcm(AsInitRecorderParam *param)
{
  param->codec_type = AS_CODECTYPE_LPCM;
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::start(void)
{
  CMN_SimpleFifoClear(&m_recorder_simple_fifo_handle);

  m_es_size = 0;

  bool result = AS_StartMediaRecorder();
  if (!result)
    {
      print_err("Error: AS_StartMediaRecorder() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  return MEDIARECORDER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::stop(void)
{
  bool result = AS_StopMediaRecorder();
  if (!result)
    {
      print_err("Error: AS_StopMediaRecorder() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  return MEDIARECORDER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::deactivate(void)
{
  bool result = AS_DeactivateMediaRecorder();
  if (!result)
    {
      print_err("Error: AS_DeactivateMediaRecorder() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  result = deactivateBaseband();
  if (!result)
    {
      print_err("Error: Baseband deactivateion failure!\n");
      return MEDIARECORDER_ECODE_BASEBAND_ERROR;
    }

  return MEDIARECORDER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::readFrames(uint8_t* p_buffer, uint32_t buffer_size, uint32_t* read_size)
{
  err_t rst = MEDIARECORDER_ECODE_OK;

  if (p_buffer == NULL)
    {
      print_err("ERROR: Buffer area not specified.\n");
      return MEDIARECORDER_ECODE_BUFFER_AREA_ERROR;
    }
  if (buffer_size == 0)
    {
      print_err("ERROR: Buffer area size error.\n");
      return MEDIARECORDER_ECODE_BUFFER_SIZE_ERROR;
    }

  size_t data_size = CMN_SimpleFifoGetOccupiedSize(&m_recorder_simple_fifo_handle);
  print_dbg("dsize = %d\n", data_size);

  *read_size = 0;
  size_t poll_size = 0;
  if (data_size > 0)
    {
      if (data_size > buffer_size)
        {
          print_err("WARNING: Insufficient buffer area.\n");
          poll_size = (size_t)buffer_size;
          rst = MEDIARECORDER_ECODE_INSUFFICIENT_BUFFER_AREA;
        }
      else
        {
          poll_size = data_size;
        }

      if (CMN_SimpleFifoPoll(&m_recorder_simple_fifo_handle, (void*)p_buffer, poll_size) == 0)
        {
          print_err("ERROR: Fail to get data from simple FIFO.\n");
          return MEDIARECORDER_ECODE_BUFFER_POLL_ERROR;
        }
      *read_size = (uint32_t)poll_size;

      m_es_size += poll_size;
    }

  return rst;
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::writeWavHeader(File& myfile)
{
  myfile.seek(0);

  m_wav_format.total_size = m_es_size + sizeof(WavFormat_t) - 8;
  m_wav_format.data_size  = m_es_size;

  int ret = myfile.write((uint8_t*)&m_wav_format, sizeof(WavFormat_t));
  if (ret < 0)
    {
      print_err("Fail to write file(wav header)\n");
      return MEDIARECORDER_ECODE_FILEACCESS_ERROR;
    }

  return MEDIARECORDER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
bool MediaRecorder::check_encode_dsp(uint8_t codec_type, const char *path, uint32_t sampling_rate)
{
  char fullpath[32];
  cxd56_audio_clkmode_t clk = CXD56_AUDIO_CLKMODE_NORMAL;
  
  switch (codec_type)
    {
      case AS_CODECTYPE_MP3:
        snprintf(fullpath, sizeof(fullpath), "%s/MP3ENC", path);
        break;

      case AS_CODECTYPE_WAV:
      case AS_CODECTYPE_LPCM:
        clk = cxd56_audio_get_clkmode();
        if (!((clk == CXD56_AUDIO_CLKMODE_NORMAL && sampling_rate != AS_SAMPLINGRATE_48000)
           || (clk == CXD56_AUDIO_CLKMODE_HIRES && sampling_rate != AS_SAMPLINGRATE_192000)))
          {
            return true;
          }
        snprintf(fullpath, sizeof(fullpath), "%s/SRC", path);
        break;

      case AS_CODECTYPE_OPUS:
        snprintf(fullpath, sizeof(fullpath), "%s/OPUSENC", path);
        break;

      default:
        break;
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

/*--------------------------------------------------------------------------*/
bool MediaRecorder::activateBaseband(void)
{
  CXD56_AUDIO_ECODE error_code;

  /* Power on audio device */

  error_code = cxd56_audio_poweron();

  if (error_code != CXD56_AUDIO_ECODE_OK)
    {
      print_err("cxd56_audio_poweron() error! [%d]\n", error_code);
      return false;
    }

  /* Enable input */

  error_code = cxd56_audio_en_input();

  if (error_code != CXD56_AUDIO_ECODE_OK)
    {
       print_err("cxd56_audio_en_input() error! [%d]\n", error_code);
       return false;
    }

  return true;
}

/*--------------------------------------------------------------------------*/
bool MediaRecorder::deactivateBaseband(void)
{
  CXD56_AUDIO_ECODE error_code;

  /* Disable input */

  error_code = cxd56_audio_dis_input();

  if (error_code != CXD56_AUDIO_ECODE_OK)
    {
      print_err("cxd56_audio_dis_input() error! [%d]\n", error_code);
      return false;
    }

  /* Power off audio device */

  error_code = cxd56_audio_poweroff();

  if (error_code != CXD56_AUDIO_ECODE_OK)
    {
      print_err("cxd56_audio_poweroff() error! [%d]\n", error_code);
      return false;
    }

  return true;
}

