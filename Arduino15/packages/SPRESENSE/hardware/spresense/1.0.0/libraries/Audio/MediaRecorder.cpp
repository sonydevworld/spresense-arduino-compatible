/*
 *  MediaRecorder.cpp - MediaRecorder implement file for the Spresense SDK
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
#include <arch/board/cxd56_audio.h>

#include "MediaRecorder.h"
#include "MemoryUtil.h"


#include <File.h>

/*--------------------------------------------------------------------------*/
void  output_device_callback(uint32_t size)
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
 * Public API on MediaRecorder Class
 ****************************************************************************/

err_t MediaRecorder::begin(void)
{
  return begin(NULL);
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::begin(AudioAttentionCb attcb)
{
  return begin(attcb, true);
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::begin(AudioAttentionCb attcb, bool use_frontend)
{
  bool result;

  if (use_frontend)
    {
      m_p_fed_ins = FrontEnd::getInstance();
    }

  if (m_p_fed_ins)
    {
      /* Create Frontend */

      err_t fed_result = m_p_fed_ins->begin();
      if (fed_result != FRONTEND_ECODE_OK)
        {
          return MEDIARECORDER_ECODE_COMMAND_ERROR;
        }
    }

  /* Create MediaRecorder feature. */

  AsCreateRecorderParams_t recorder_create_param;

  recorder_create_param.msgq_id.recorder = MSGQ_AUD_RECORDER; 
  recorder_create_param.msgq_id.mng      = MSGQ_AUD_MGR;
  recorder_create_param.msgq_id.dsp      = MSGQ_AUD_DSP;
  recorder_create_param.pool_id.input    = S0_MIC_IN_BUF_POOL;
  recorder_create_param.pool_id.output   = S0_OUTPUT_BUF_POOL;
  recorder_create_param.pool_id.dsp      = S0_ENC_APU_CMD_POOL;

  result = AS_CreateMediaRecorder(&recorder_create_param, (attcb) ? attcb : attentionCallback);
  if (!result)
    {
      print_err("Error: AS_CreateMediaRecorder() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  return MEDIARECORDER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::end(void)
{
  bool result;

  if (m_p_fed_ins)
    {
      /* Delete Frontend */

      err_t fed_result = m_p_fed_ins->end();
      if (fed_result != FRONTEND_ECODE_OK)
        {
          return MEDIARECORDER_ECODE_COMMAND_ERROR;
        }
    }

  /* Delete MediaRecorder */

  result = AS_DeleteMediaRecorder();
  if (!result)
    {
      print_err("Error: AS_DeleteMediaRecorder() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  return MEDIARECORDER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::activate(AsSetRecorderStsInputDevice input_device, MediaRecorderCallback mrcb)
{
  return activate(input_device, mrcb, MEDIARECORDER_BUF_SIZE);
}
 
/*--------------------------------------------------------------------------*/
err_t MediaRecorder::activate(AsSetRecorderStsInputDevice input_device,
                              MediaRecorderCallback mrcb,
                              uint32_t recorder_bufsize)
{
  return activate(input_device, mrcb, recorder_bufsize, AsMicFrontendPreProcThrough);
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::activate(AsSetRecorderStsInputDevice input_device,
                              MediaRecorderCallback mrcb,
                              uint32_t recorder_bufsize,
                              AsMicFrontendPreProcType proc_type,
                              bool is_digital)
{
  /* Hold callback */

  m_mr_callback = mrcb;

  /* Buffer size check */

  if (!recorder_bufsize)
    {
      print_err("Invalid buffer size.\n");
      return MEDIARECORDER_ECODE_BUFFER_SIZE_ERROR;
    }

  /* Allocate ES buffer */

  m_recorder_simple_fifo_buf = static_cast<uint32_t *>(malloc(recorder_bufsize));

  if (!m_recorder_simple_fifo_buf)
    {
      print_err("Buffer allocate error.\n");
      return MEDIARECORDER_ECODE_BUFFER_ALLOC_ERROR;
    }

  if (CMN_SimpleFifoInitialize(&m_recorder_simple_fifo_handle,
                               m_recorder_simple_fifo_buf,
                               recorder_bufsize,
                               NULL) != 0)
    {
      print_err("Fail to initialize simple FIFO.\n");
      return MEDIARECORDER_ECODE_BUFFER_INIT_ERROR;
    }

  CMN_SimpleFifoClear(&m_recorder_simple_fifo_handle);

  bool result;

  if (m_p_fed_ins)
    {
      /* Activate Frontend (sync move) */

      err_t fed_result = m_p_fed_ins->activate(NULL, is_digital);
      if (fed_result != FRONTEND_ECODE_OK)
        {
          m_mr_callback(AsRecorderEventAct, fed_result, 0);
          return MEDIARECORDER_ECODE_COMMAND_ERROR;
        }
    }

  /* Activate MediaRecorder */

  AsActivateRecorder recorder_act;

  m_output_device_handler.simple_fifo_handler = (void*)(&m_recorder_simple_fifo_handle);
  m_output_device_handler.callback_function   = output_device_callback;

  recorder_act.param.input_device          = input_device;
  recorder_act.param.output_device         = AS_SETRECDR_STS_OUTPUTDEVICE_RAM;
  recorder_act.param.input_device_handler  = 0x00;
  recorder_act.param.output_device_handler = &m_output_device_handler;
  recorder_act.cb                          = NULL;

  result = AS_ActivateMediaRecorder(&recorder_act);
  if (!result)
    {
      print_err("Error: AS_ActivateMediaRecorder() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  AudioObjReply reply_info;
  result = AS_ReceiveObjectReply(MSGQ_AUD_MGR, &reply_info);
  if (!result)
    {
      print_err("Error: AS_ReceiveObjectReply() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  if (reply_info.result != AS_ECODE_OK)
    {
      m_mr_callback(AsRecorderEventAct, reply_info.result, 0);
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  m_mr_callback(AsRecorderEventAct, AS_ECODE_OK, 0);

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

  bool result;


  if (m_p_fed_ins)
    {
      /* Init Frontend */

      AsDataDest dest;
      dest.msg.msgqid  = MSGQ_AUD_RECORDER;
      dest.msg.msgtype = MSG_AUD_MRC_CMD_ENCODE;

      err_t fed_result = m_p_fed_ins->init(channel_number,
                                           bit_length,
                                           getCapSampleNumPerFrame(codec_type, sampling_rate),
                                           AsDataPathMessage,
                                           dest);
      if (fed_result != FRONTEND_ECODE_OK)
        {
          m_mr_callback(AsRecorderEventInit, fed_result, 0);
          return MEDIARECORDER_ECODE_COMMAND_ERROR;
        }
    } 

  /* Init MediaRecorder */

  AsInitRecorderParam init_param;

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

  AudioObjReply reply_info;
  result = AS_ReceiveObjectReply(MSGQ_AUD_MGR, &reply_info);
  if (!result)
    {
      print_err("Error: AS_ReceiveObjectReply() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  m_mr_callback(AsRecorderEventInit, reply_info.result, 0);

  return MEDIARECORDER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
void MediaRecorder::init_wav(AsInitRecorderParam *param)
{
  param->codec_type = AS_CODECTYPE_LPCM;

  /* Create WAV header information */

  m_wav_format.riff     = CHUNKID_RIFF;
  m_wav_format.wave     = FORMAT_WAVE;
  m_wav_format.fmt      = SUBCHUNKID_FMT;
  m_wav_format.fmt_size = FMT_CHUNK_SIZE;
  m_wav_format.format   = FORMAT_ID_PCM;
  m_wav_format.channel  = param->channel_number;
  m_wav_format.rate     = param->sampling_rate;
  m_wav_format.avgbyte  = param->sampling_rate * param->channel_number * (param->bit_length / 8);
  m_wav_format.block    = param->channel_number * (param->bit_length / 8);
  m_wav_format.bit      = param->bit_length;
  m_wav_format.data     = SUBCHUNKID_DATA;
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
  if (m_recorder_simple_fifo_buf == NULL)
    {
      print_err("ERROR: FIFO area is not allcated.\n");
      return MEDIARECORDER_ECODE_BUFFER_AREA_ERROR;
    }

  CMN_SimpleFifoClear(&m_recorder_simple_fifo_handle);

  m_es_size = 0;

  bool result;

  if (m_p_fed_ins)
    {
      /* Start Frontend */

      err_t fed_result = m_p_fed_ins->start();
      if (fed_result != FRONTEND_ECODE_OK)
        {
          m_mr_callback(AsRecorderEventInit, fed_result, 0);
          return MEDIARECORDER_ECODE_COMMAND_ERROR;
        }
    }

  /* Start MediaRecorder */

  result = AS_StartMediaRecorder();
  if (!result)
    {
      print_err("Error: AS_StartMediaRecorder() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  AudioObjReply reply_info;
  result = AS_ReceiveObjectReply(MSGQ_AUD_MGR, &reply_info);
  if (!result)
    {
      print_err("Error: AS_ReceiveObjectReply() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  m_mr_callback(AsRecorderEventStart, reply_info.result, 0);

  return MEDIARECORDER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::stop(void)
{
  bool result;

  if (m_p_fed_ins)
    {
      /* Stop Frontend */

      err_t fed_result = m_p_fed_ins->stop();
      if (fed_result != FRONTEND_ECODE_OK)
        {
          m_mr_callback(AsRecorderEventInit, fed_result, 0);
          return MEDIARECORDER_ECODE_COMMAND_ERROR;
        }
    }

  /* Stop MediaRecorder */

  result = AS_StopMediaRecorder();
  if (!result)
    {
      print_err("Error: AS_StopMediaRecorder() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  AudioObjReply reply_info;
  result = AS_ReceiveObjectReply(MSGQ_AUD_MGR, &reply_info);
  if (!result)
    {
      print_err("Error: AS_ReceiveObjectReply() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  m_mr_callback(AsRecorderEventStop, reply_info.result, 0);

  return MEDIARECORDER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::deactivate(void)
{
  bool result;

  if (m_p_fed_ins)
    {
      /* Deactivate frontend */

      err_t fed_result = m_p_fed_ins->deactivate();
      if (fed_result != FRONTEND_ECODE_OK)
        {
          m_mr_callback(AsRecorderEventInit, fed_result, 0);
          return MEDIARECORDER_ECODE_COMMAND_ERROR;
        }
    }

  /* Deactivate MediaRecorder */

  result = AS_DeactivateMediaRecorder();
  if (!result)
    {
      print_err("Error: AS_DeactivateMediaRecorder() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  AudioObjReply reply_info;
  result = AS_ReceiveObjectReply(MSGQ_AUD_MGR, &reply_info);
  if (!result)
    {
      print_err("Error: AS_ReceiveObjectReply() failure!\n");
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  if (reply_info.result != AS_ECODE_OK)
    {
      m_mr_callback(AsRecorderEventDeact, reply_info.result, 0);
      return MEDIARECORDER_ECODE_COMMAND_ERROR;
    }

  /* Free ES buffer */

  free((void*)m_recorder_simple_fifo_buf);
  m_recorder_simple_fifo_buf = NULL;

  m_mr_callback(AsRecorderEventDeact, AS_ECODE_OK, 0);

  return MEDIARECORDER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t MediaRecorder::setMicGain(int16_t mic_gain)
{
  if (m_p_fed_ins)
    {
      /* Set Mic Gain */

      err_t fed_result = m_p_fed_ins->setMicGain(mic_gain);
      if (fed_result != FRONTEND_ECODE_OK)
        {
          return MEDIARECORDER_ECODE_COMMAND_ERROR;
        }
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

  if (m_recorder_simple_fifo_buf == NULL)
    {
      print_err("ERROR: FIFO area is not allcated.\n");
      return MEDIARECORDER_ECODE_BUFFER_AREA_ERROR;
    }

  size_t data_size = CMN_SimpleFifoGetOccupiedSize(&m_recorder_simple_fifo_handle);

  *read_size = 0;
  size_t poll_size = 0;
  if (data_size > 0)
    {
      if (data_size > buffer_size)
        {
          print_dbg("WARNING: Insufficient buffer area.\n");
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

  m_wav_format.total_size = m_es_size + sizeof(WAVHEADER) - 8;
  m_wav_format.data_size  = m_es_size;

  int ret = myfile.write((uint8_t*)&m_wav_format, sizeof(WAVHEADER));
  if (ret != sizeof(WAVHEADER))
    {
      print_err("Fail to write file(wav header)\n");
      return MEDIARECORDER_ECODE_FILEACCESS_ERROR;
    }

  m_es_size = 0;

  return MEDIARECORDER_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
bool MediaRecorder::check_encode_dsp(uint8_t codec_type, const char *path, uint32_t sampling_rate)
{
  char fullpath[32] = { 0 };
  cxd56_audio_clkmode_t clk = CXD56_AUDIO_CLKMODE_NORMAL;
  struct stat buf;
  int retry;
  int ret = 0;
  
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

/*--------------------------------------------------------------------------*/
bool MediaRecorder::setCapturingClkMode(uint8_t clk_mode)
{
  if (m_p_fed_ins)
    {
      err_t fed_result = m_p_fed_ins->setCapturingClkMode(clk_mode);
      return (fed_result == FRONTEND_ECODE_OK) ? true : false;
    }

  return false;
}



