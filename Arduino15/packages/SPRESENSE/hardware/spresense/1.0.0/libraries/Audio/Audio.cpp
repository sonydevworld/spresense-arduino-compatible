/*
 *  Audio.cpp - SPI implement file for the Spresense SDK
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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <nuttx/init.h>
#include <nuttx/arch.h>
#include <arch/chip/pm.h>
#include <arch/board/board.h>

#include "Audio.h"
#include "MemoryUtil.h"

//***************************************************************************
// Definitions
//***************************************************************************
// Configuration ************************************************************
// C++ initialization requires CXX initializer support

#include <asmp/mpshm.h>

#include "memutil/msgq_id.h"
#include "memutil/mem_layout.h"
#include "memutil/memory_layout.h"

extern "C" void  input_device_callback(uint32_t);
extern "C" void  output_device_callback(uint32_t);

/****************************************************************************
 * Common API on Audio Class
 ****************************************************************************/
err_t AudioClass::begin(void)
{
  int ret;

  ret = begin_manager();
  if (ret != AUDIOLIB_ECODE_OK)
    {
      print_err("Audio activation error.\n");
      return ret;
    }

  ret = begin_player();
  if (ret != AUDIOLIB_ECODE_OK)
    {
      print_err("Player creation error.\n");
      return ret;
    }

  ret = begin_recorder();
  if (ret != AUDIOLIB_ECODE_OK)
    {
      print_err("Recorder creation error.\n");
      return ret;
    }

  return ret;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::end(void)
{
  end_player();
  end_recorder();
  end_manager();

  return AUDIOLIB_ECODE_OK;
}

/****************************************************************************
 * Private Common API for begin/end
 ****************************************************************************/
extern "C" {

void attentionCallback(const ErrorAttentionParam *attparam)
{
  print_dbg("Attention!! Level 0x%x Code 0x%x\n", attparam->error_code, attparam->error_att_sub_code);
}

}

/*--------------------------------------------------------------------------*/
err_t AudioClass::begin_manager(void)
{
  int ret;

  ret = initMemoryPools();
  if (ret != AUDIOLIB_ECODE_OK)
    {
      print_err("Memory pool initilization error.\n");
      return ret;
    }

  ret = activateAudio();
  if (ret != AUDIOLIB_ECODE_OK)
    {
      print_err("Audio activation error.\n");
      return ret;
    }

  return ret;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::begin_player(void)
{
  AsCreatePlayerParam_t player_create_param;
  player_create_param.msgq_id.player = MSGQ_AUD_PLY;
  player_create_param.msgq_id.mng    = MSGQ_AUD_MGR;
  player_create_param.msgq_id.mixer  = MSGQ_AUD_OUTPUT_MIX;
  player_create_param.msgq_id.dsp    = MSGQ_AUD_DSP;
  player_create_param.pool_id.es     = DEC_ES_MAIN_BUF_POOL;
  player_create_param.pool_id.pcm    = REND_PCM_BUF_POOL;
  player_create_param.pool_id.dsp    = DEC_APU_CMD_POOL;

  int act_rst = AS_CreatePlayer(AS_PLAYER_ID_0, &player_create_param);
  if (!act_rst)
    {
      print_err("AS_CreatePlayer failed. system memory insufficient!\n");
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  player_create_param.msgq_id.player = MSGQ_AUD_SUB_PLY;
  player_create_param.msgq_id.mng    = MSGQ_AUD_MGR;
  player_create_param.msgq_id.mixer  = MSGQ_AUD_OUTPUT_MIX;
  player_create_param.msgq_id.dsp    = MSGQ_AUD_DSP;
  player_create_param.pool_id.es     = DEC_ES_SUB_BUF_POOL;
  player_create_param.pool_id.pcm    = REND_PCM_SUB_BUF_POOL;
  player_create_param.pool_id.dsp    = DEC_APU_CMD_POOL;

  act_rst = AS_CreatePlayer(AS_PLAYER_ID_1, &player_create_param);
  if (!act_rst)
    {
      print_err("AS_CreatePlayer failed. system memory insufficient!\n");
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  AsCreateOutputMixParam_t output_mix_create_param;

  output_mix_create_param.msgq_id.mixer = MSGQ_AUD_OUTPUT_MIX;
  output_mix_create_param.msgq_id.render_path0_filter_dsp = MSGQ_AUD_PFDSP0;
  output_mix_create_param.msgq_id.render_path1_filter_dsp = MSGQ_AUD_PFDSP1;
  output_mix_create_param.pool_id.render_path0_filter_pcm = PF0_PCM_BUF_POOL;
  output_mix_create_param.pool_id.render_path1_filter_pcm = PF1_PCM_BUF_POOL;
  output_mix_create_param.pool_id.render_path0_filter_dsp = PF0_APU_CMD_POOL;
  output_mix_create_param.pool_id.render_path1_filter_dsp = PF1_APU_CMD_POOL;

  act_rst = AS_CreateOutputMixer(&output_mix_create_param);
  if (!act_rst)
    {
      print_err("AS_CreateOutputMix failed. system memory insufficient!\n");
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  AsCreateRendererParam_t renderer_act_param;
  renderer_act_param.msgq_id.dev0_req  = MSGQ_AUD_RND_PLY;
  renderer_act_param.msgq_id.dev0_sync = MSGQ_AUD_RND_PLY_SYNC;
  renderer_act_param.msgq_id.dev1_req   = MSGQ_AUD_RND_SUB;
  renderer_act_param.msgq_id.dev1_sync  = MSGQ_AUD_RND_SUB_SYNC;

  act_rst = AS_CreateRenderer(&renderer_act_param);
  if (!act_rst)
    {
      print_err("AS_CreateRenderer failed. system memory insufficient!\n");
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  print_dbg("cmplt Activation\n");

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::begin_recorder(void)
{
  AsCreateRecorderParam_t recorder_act_param;
  recorder_act_param.msgq_id.recorder      = MSGQ_AUD_RECORDER;
  recorder_act_param.msgq_id.mng           = MSGQ_AUD_MGR;
  recorder_act_param.msgq_id.dsp           = MSGQ_AUD_DSP;
  recorder_act_param.pool_id.input         = MIC_IN_BUF_POOL;
  recorder_act_param.pool_id.output        = OUTPUT_BUF_POOL;
  recorder_act_param.pool_id.dsp           = ENC_APU_CMD_POOL;

  if (!AS_CreateMediaRecorder(&recorder_act_param))
    {
      print_err("AS_CreateMediaRecorder failed. system memory insufficient!\n");
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  AsCreateCaptureParam_t capture_act_param;
  capture_act_param.msgq_id.dev0_req  = MSGQ_AUD_CAP;
  capture_act_param.msgq_id.dev0_sync = MSGQ_AUD_CAP_SYNC;
  capture_act_param.msgq_id.dev1_req  = 0xFF;
  capture_act_param.msgq_id.dev1_sync = 0xFF;

  if (!AS_CreateCapture(&capture_act_param))
    {
      print_err("AS_CreateCapture failed. system memory insufficient!\n");
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::end_manager(void)
{
  AS_DeleteAudioManager();

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::end_player(void)
{
  AS_DeletePlayer(AS_PLAYER_ID_0);
  AS_DeletePlayer(AS_PLAYER_ID_1);
  AS_DeleteOutputMix();
  AS_DeleteRenderer();

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::end_recorder(void)
{
  AS_DeleteMediaRecorder();
  AS_DeleteCapture();

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::activateAudio(void)
{
  AudioSubSystemIDs ids;
  int ret = AUDIOLIB_ECODE_OK;

  ids.app         = MSGQ_AUD_APP;
  ids.mng         = MSGQ_AUD_MGR;
  ids.player_main = MSGQ_AUD_PLY;
  ids.player_sub  = MSGQ_AUD_SUB_PLY;
  ids.mixer       = MSGQ_AUD_OUTPUT_MIX;
  ids.recorder    = MSGQ_AUD_RECORDER;
  ids.effector    = 0xFF;
  ids.recognizer  = 0xFF;

  AS_CreateAudioManager(ids, attentionCallback);

  ret = powerOn();
  if (ret != AUDIOLIB_ECODE_OK)
    {
      print_err("Power On error.\n");
      return ret;
    }

  return ret;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::powerOn(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_POWERON;
  command.header.command_code  = AUDCMD_POWERON;
  command.header.sub_code      = 0x00;
  command.power_on_param.enable_sound_effect = AS_DISABLE_SOUNDEFFECT;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_STATUSCHANGED)
    {
      print_err("ERROR: Command (%x) fails. Result code(%x)\n", command.header.command_code, result.header.result_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

	print_dbg("power on!\n");

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::powerOff(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SET_POWEROFF_STATUS;
  command.header.command_code  = AUDCMD_SETPOWEROFFSTATUS;
  command.header.sub_code      = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_STATUSCHANGED)
    {
      print_err("ERROR: Command (%x) fails. Result code(%x)\n", command.header.command_code, result.header.result_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::setReadyMode(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SET_READY_STATUS;
  command.header.command_code  = AUDCMD_SETREADYSTATUS;
  command.header.sub_code      = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_STATUSCHANGED)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
              command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  board_external_amp_mute_control(true);

  destroyStaticPools();

  return AUDIOLIB_ECODE_OK;
}
/****************************************************************************
 * Player API on Audio Class
 ****************************************************************************/
err_t AudioClass::setPlayerMode(uint8_t device)
{
  const NumLayout layout_no = MEM_LAYOUT_PLAYER;

  assert(layout_no < NUM_MEM_LAYOUTS);
  createStaticPools(layout_no);

  AudioClass::set_output(device);

  print_dbg("set output cmplt\n");

  if (CMN_SimpleFifoInitialize(&m_player0_simple_fifo_handle, m_player0_simple_fifo_buf, SIMPLE_FIFO_BUF_SIZE, NULL) != 0)
    {
      print_err("Fail to initialize simple FIFO.\n");
      return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
    }
  CMN_SimpleFifoClear(&m_player0_simple_fifo_handle);

  if (CMN_SimpleFifoInitialize(&m_player1_simple_fifo_handle, m_player1_simple_fifo_buf, WRITE_BUF_SIZE, NULL) != 0)
    {
      print_err("Fail to initialize simple FIFO.\n");
      return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
    }
  CMN_SimpleFifoClear(&m_player1_simple_fifo_handle);

  m_player0_input_device_handler.simple_fifo_handler = (void*)(&m_player0_simple_fifo_handle);
  m_player0_input_device_handler.callback_function = input_device_callback; /*??*/

  m_player1_input_device_handler.simple_fifo_handler = (void*)(&m_player1_simple_fifo_handle);
  m_player1_input_device_handler.callback_function = input_device_callback; /*??*/

  AudioCommand command;
  command.header.packet_length = LENGTH_SET_PLAYER_STATUS;
  command.header.command_code  = AUDCMD_SETPLAYERSTATUS;
  command.header.sub_code      = 0x00;

  command.set_player_sts_param.active_player         = AS_ACTPLAYER_BOTH;
  command.set_player_sts_param.player0.input_device  = AS_SETPLAYER_INPUTDEVICE_RAM;
  command.set_player_sts_param.player0.ram_handler   = &m_player0_input_device_handler;
  command.set_player_sts_param.player0.output_device = device;
  command.set_player_sts_param.player1.input_device  = AS_SETPLAYER_INPUTDEVICE_RAM;
  command.set_player_sts_param.player1.ram_handler   = &m_player1_input_device_handler;
  command.set_player_sts_param.player1.output_device = device;

  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_STATUSCHANGED)
    {
      print_err("ERROR: Command (%x) fails. Result code(%x), subcode = %x\n", command.header.command_code, result.header.result_code,result.error_response_param.error_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  board_external_amp_mute_control(false);

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::initPlayer(PlayerId id, uint8_t codec_type,
                             uint32_t sampling_rate, uint8_t channel_number)
{
  return initPlayer(id, codec_type, "/mnt/sd0/BIN", sampling_rate, AS_BITLENGTH_16, channel_number);
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::initPlayer(PlayerId id, uint8_t codec_type,
                             uint32_t sampling_rate, uint8_t bit_length, uint8_t channel_number)
{
  return initPlayer(id, codec_type, "/mnt/sd0/BIN", sampling_rate, bit_length, channel_number);
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::initPlayer(PlayerId id, uint8_t codec_type, const char* codec_path,
                             uint32_t sampling_rate, uint8_t channel_number)
{
  return initPlayer(id, codec_type, codec_path, sampling_rate, AS_BITLENGTH_16, channel_number);
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::initPlayer(PlayerId id, uint8_t codec_type, const char* codec_path,
                             uint32_t sampling_rate, uint8_t bit_length, uint8_t channel_number)
{
  if (!check_decode_dsp(codec_type, codec_path))
    {
      return AUDIOLIB_ECODE_FILEACCESS_ERROR;
    }

  AudioCommand command;

  command.header.packet_length = LENGTH_INIT_PLAYER;
  command.header.command_code  = AUDCMD_INITPLAYER;
  command.header.sub_code      = 0x00;

  command.player.player_id = (id == Player0) ? AS_PLAYER_ID_0 : AS_PLAYER_ID_1;
  command.player.init_param.codec_type    = codec_type;
  command.player.init_param.bit_length    = bit_length;
  command.player.init_param.channel_number= channel_number;
  command.player.init_param.sampling_rate = sampling_rate;
  snprintf(command.player.init_param.dsp_path, AS_AUDIO_DSP_PATH_LEN, "%s", codec_path);
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_INITPLAYERCMPLT)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
              command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::startPlayer(PlayerId id)
{

  AudioCommand command;

  command.header.packet_length = LENGTH_PLAY_PLAYER;
  command.header.command_code  = AUDCMD_PLAYPLAYER;
  command.header.sub_code      = 0x00;

  command.player.player_id = (id == Player0) ? AS_PLAYER_ID_0 : AS_PLAYER_ID_1;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_PLAYCMPLT)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x) Error subcode(0x%x)\n",
              command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code, result.error_response_param.error_sub_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::setBeep(char en, short vol, short freq)
{
  AudioCommand command;

  command.header.packet_length = LENGTH_SETBEEPPARAM;
  command.header.command_code  = AUDCMD_SETBEEPPARAM;
  command.header.sub_code      = 0;

  command.set_beep_param.beep_en   = en;
  command.set_beep_param.beep_vol  = vol;
  command.set_beep_param.beep_freq = freq;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_SETBEEPCMPLT)
    {
      print_err("ERROR: Command (%x) fails. Result code(%x) Error code(0x%x)\n",
                command.header.command_code, result.header.result_code, result.error_response_param.error_code);

      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::stopPlayer(PlayerId id)
{
  return stopPlayer(id, AS_STOPPLAYER_ESEND);
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::stopPlayer(PlayerId id, uint8_t mode)
{
  AudioCommand command;

  command.header.packet_length = LENGTH_STOP_PLAYER;
  command.header.command_code  = AUDCMD_STOPPLAYER;
  command.header.sub_code      = 0x00;

  command.player.player_id = (id == Player0) ? AS_PLAYER_ID_0 : AS_PLAYER_ID_1;
  command.player.stop_param.stop_mode = mode;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_STOPCMPLT)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
              command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  CMN_SimpleFifoHandle *handle = (id == Player0) ? &m_player0_simple_fifo_handle : &m_player1_simple_fifo_handle;
  CMN_SimpleFifoClear(handle);

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::setVolume(int master_db)
{
  AudioCommand command;

  command.header.packet_length = LENGTH_SETVOLUME;
  command.header.command_code  = AUDCMD_SETVOLUME;
  command.header.sub_code      = 0;

  command.set_volume_param.input1_db = 0; /* 0dB */
  command.set_volume_param.input2_db = 0; /* 0dB */
  command.set_volume_param.master_db = master_db;

  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_SETVOLUMECMPLT)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
              command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::setVolume(int master, int player0, int player1)
{
  AudioCommand command;

  command.header.packet_length = LENGTH_SETVOLUME;
  command.header.command_code  = AUDCMD_SETVOLUME;
  command.header.sub_code      = 0;

  command.set_volume_param.input1_db = player0;
  command.set_volume_param.input2_db = player1;
  command.set_volume_param.master_db = master;

  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_SETVOLUMECMPLT)
    {
      printf("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
             command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::setLRgain(PlayerId id, unsigned char l_gain, unsigned char r_gain)
{
  AudioCommand command;

  command.header.packet_length = LENGTH_SET_GAIN;
  command.header.command_code  = AUDCMD_SETGAIN;
  command.header.sub_code      = 0;

  command.player.player_id = (id == Player0) ? AS_PLAYER_ID_0 : AS_PLAYER_ID_1;
  command.player.set_gain_param.l_gain = l_gain;
  command.player.set_gain_param.r_gain = r_gain;

  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_SETGAIN_CMPLT)
    {
      printf("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
              command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

#define WRITE_FRAME_NUM 5 
/*--------------------------------------------------------------------------*/
err_t AudioClass::writeFrames(PlayerId id, int fd)
{
  int ret = AUDIOLIB_ECODE_OK;
  char *buf = (id == Player0) ? m_es_player0_buf : m_es_player1_buf; 
  CMN_SimpleFifoHandle *handle = (id == Player0) ? &m_player0_simple_fifo_handle : &m_player1_simple_fifo_handle;
  uint32_t write_size = (id == Player0) ? FIFO_FRAME_SIZE : WRITE_FIFO_FRAME_SIZE;

  for (int i = 0; i < WRITE_FRAME_NUM; i++)
    {
        ret = write_fifo(fd, buf, write_size, handle);
      if(ret != AUDIOLIB_ECODE_OK) break;
    }

    return ret;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::writeFrames(PlayerId id, File& myFile)
{
  int ret = AUDIOLIB_ECODE_OK;
  char *buf = (id == Player0) ? m_es_player0_buf : m_es_player1_buf; 
  CMN_SimpleFifoHandle *handle = (id == Player0) ? &m_player0_simple_fifo_handle : &m_player1_simple_fifo_handle;
  uint32_t write_size = (id == Player0) ? FIFO_FRAME_SIZE : WRITE_FIFO_FRAME_SIZE;

  for (int i = 0; i < WRITE_FRAME_NUM; i++)
    {
      ret = write_fifo(myFile, buf, write_size, handle);
      if (ret != AUDIOLIB_ECODE_OK) break;
    }

  return ret;
}


/****************************************************************************
 * Recoder API on Audio Class
 ****************************************************************************/
#define m_recorder_simple_fifo_handle m_player0_simple_fifo_handle
#define m_recorder_simple_fifo_buf m_player0_simple_fifo_buf
#define m_es_recorder_buf m_es_player0_buf

err_t AudioClass::setRecorderMode(uint8_t input_device, int32_t gain)
{
  const NumLayout layout_no = MEM_LAYOUT_RECORDER;

  assert(layout_no < NUM_MEM_LAYOUTS);
  createStaticPools(layout_no);

  if (CMN_SimpleFifoInitialize(&m_recorder_simple_fifo_handle, m_recorder_simple_fifo_buf, SIMPLE_FIFO_BUF_SIZE, NULL) != 0)
    {
      print_err("Fail to initialize simple FIFO.\n");
      return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
    }

  CMN_SimpleFifoClear(&m_recorder_simple_fifo_handle);

  m_output_device_handler.simple_fifo_handler = (void*)(&m_recorder_simple_fifo_handle);
  m_output_device_handler.callback_function = output_device_callback;

  AudioCommand command;

  command.header.packet_length = LENGTH_SET_RECORDER_STATUS;
  command.header.command_code = AUDCMD_SETRECORDERSTATUS;
  command.header.sub_code = 0x00;

  command.set_recorder_status_param.input_device = input_device;
  command.set_recorder_status_param.input_device_handler = 0x00;
  command.set_recorder_status_param.output_device = AS_SETRECDR_STS_OUTPUTDEVICE_RAM;
  command.set_recorder_status_param.output_device_handler = &m_output_device_handler;

  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_STATUSCHANGED)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
              command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  if (init_mic_gain(input_device, gain) != AUDIOLIB_ECODE_OK)
    {
      print_err("Mic init error!");
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::setRecorderMode(uint8_t input_device)
{
	return setRecorderMode(input_device,0);
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::init_recorder_wav(AudioCommand* command, uint32_t sampling_rate, uint8_t bit_length, uint8_t channel_number)
{
  command->recorder.init_param.sampling_rate  = sampling_rate;
  command->recorder.init_param.channel_number = channel_number;
  command->recorder.init_param.bit_length     = bit_length;
  command->recorder.init_param.codec_type     = AS_CODECTYPE_PCM;
  AS_SendAudioCommand(command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  if (result.header.result_code != AUDRLT_INITRECCMPLT)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
                command->header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  memcpy(m_wav_format.riff, CHUNKID_RIFF, strlen(CHUNKID_RIFF));
  memcpy(m_wav_format.wave, FORMAT_WAVE, strlen(FORMAT_WAVE));
  memcpy(m_wav_format.fmt, SUBCHUNKID_FMT, strlen(SUBCHUNKID_FMT));
  m_wav_format.fmt_size = FMT_SIZE;
  m_wav_format.format   = AUDIO_FORMAT_PCM;
  m_wav_format.channel  = channel_number;
  m_wav_format.rate     = sampling_rate;
  m_wav_format.avgbyte  = sampling_rate * channel_number * 2;
  m_wav_format.block    = channel_number * 2;
  m_wav_format.bit      = 2 * 8;
  memcpy(m_wav_format.data, SUBCHUNKID_DATA, strlen(SUBCHUNKID_DATA));

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::init_recorder_mp3(AudioCommand* command, uint32_t sampling_rate, uint8_t bit_length, uint8_t channel_number)
{
  command->recorder.init_param.sampling_rate  = sampling_rate;
  command->recorder.init_param.channel_number = channel_number;
  command->recorder.init_param.bit_length     = bit_length;
  command->recorder.init_param.codec_type     = m_codec_type;
  command->recorder.init_param.bitrate        = AS_BITRATE_96000;
  AS_SendAudioCommand(command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  if (result.header.result_code != AUDRLT_INITRECCMPLT)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
                command->header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::init_recorder_opus(AudioCommand* command, uint32_t sampling_rate, uint8_t bit_length, uint8_t channel_number)
{
  command->recorder.init_param.sampling_rate  = sampling_rate;
  command->recorder.init_param.channel_number = channel_number;
  command->recorder.init_param.bit_length     = bit_length;
  command->recorder.init_param.codec_type     = m_codec_type;
  command->recorder.init_param.bitrate        = AS_BITRATE_8000;
  command->recorder.init_param.computational_complexity = AS_INITREC_COMPLEXITY_0;
  AS_SendAudioCommand(command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  if (result.header.result_code != AUDRLT_INITRECCMPLT)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
                command->header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::init_recorder_pcm(AudioCommand* command, uint32_t sampling_rate, uint8_t bit_length, uint8_t channel_number)
{
  command->recorder.init_param.sampling_rate  = sampling_rate;
  command->recorder.init_param.channel_number = channel_number;
  command->recorder.init_param.bit_length     = bit_length;
  command->recorder.init_param.codec_type     = m_codec_type;
  AS_SendAudioCommand(command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  if (result.header.result_code != AUDRLT_INITRECCMPLT)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
                command->header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::initRecorder(uint8_t codec_type, uint32_t sampling_rate, uint8_t channel)
{
  return initRecorder(codec_type, "/mnt/sd0/BIN", sampling_rate, AS_BITLENGTH_16, channel);
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::initRecorder(uint8_t codec_type, uint32_t sampling_rate, uint8_t bit_length, uint8_t channel)
{
  return initRecorder(codec_type, "/mnt/sd0/BIN", sampling_rate, bit_length, channel);
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::initRecorder(uint8_t codec_type, const char *codec_path,
                               uint32_t sampling_rate, uint8_t channel)
{
  return initRecorder(codec_type, codec_path, sampling_rate, AS_BITLENGTH_16, channel);
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::initRecorder(uint8_t codec_type, const char *codec_path,
                               uint32_t sampling_rate, uint8_t bit_length, uint8_t channel)
{
  
  if (!check_encode_dsp(codec_type, codec_path, sampling_rate))
    {
      return AUDIOLIB_ECODE_FILEACCESS_ERROR;
    }

  AudioCommand command;

  command.header.packet_length = LENGTH_INIT_RECORDER;
  command.header.command_code  = AUDCMD_INITREC;
  command.header.sub_code      = 0x00;
  snprintf(command.recorder.init_param.dsp_path, AS_AUDIO_DSP_PATH_LEN, "%s", codec_path);

  m_codec_type = codec_type;

  int ret = AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
  switch (codec_type)
    {
      case AS_CODECTYPE_WAV:
        ret = init_recorder_wav(&command, sampling_rate, bit_length, channel);
        break;

      case AS_CODECTYPE_MP3:
        ret = init_recorder_mp3(&command, sampling_rate, bit_length, channel);
        break;

      case AS_CODECTYPE_OPUS:
        ret = init_recorder_opus(&command, sampling_rate, bit_length, channel);
        break;

      case AS_CODECTYPE_PCM:
        ret = init_recorder_pcm(&command, sampling_rate, bit_length, channel);
        break;

      default:
        break;
    }

  return ret;
}


/*--------------------------------------------------------------------------*/
err_t AudioClass::startRecorder(void)
{
  m_es_size = 0;

  CMN_SimpleFifoClear(&m_recorder_simple_fifo_handle);

  AudioCommand command;

  command.header.packet_length = LENGTH_START_RECORDER;
  command.header.command_code  = AUDCMD_STARTREC;
  command.header.sub_code      = 0x00;

  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_RECCMPLT)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x) Error subcode(0x%x)\n",
              command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code, result.error_response_param.error_sub_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  print_dbg("start\n");

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::stopRecorder(void)
{
  AudioCommand command;

  command.header.packet_length = LENGTH_STOP_RECORDER;
  command.header.command_code  = AUDCMD_STOPREC;
  command.header.sub_code      = 0x00;

  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_STOPRECCMPLT)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
              command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
#if 0
err_t AudioClass::closeOutputFile(int fd)
{
  do
    {
    } while (read_frames(fd) == 0);

  if (m_codec_type == AS_CODECTYPE_WAV)
    {
      writeWavHeader(fd);
    }

  fclose(fd);

  return true;
}
#endif 

/*--------------------------------------------------------------------------*/
err_t AudioClass::closeOutputFile(File& myFile)
{
  do
    {
      readFrames(myFile);
    } while (CMN_SimpleFifoGetOccupiedSize(&m_recorder_simple_fifo_handle) != 0);


  if (m_codec_type == AS_CODECTYPE_WAV)
    {
      writeWavHeader(myFile);
    }

  myFile.close();

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
/*err_t AudioClass::writeWavHeader(int fd)
{
  ssize_t ret;

  m_wav_format.total_size = m_es_size + sizeof(WavaFormat_t) - 8;
  m_wav_format.data_size = m_es_size;
  fseek(fd, 0, SEEK_SET);

  int ret = fwrite(&m_wav_format, 1, sizeof(WavaFormat_t), fd);
  if (ret < 0)
    {
      print_err("Fail to write file(wav header)\n");
      return false;
    }

  return AUDIOLIB_ECODE_OK;
}*/
/*--------------------------------------------------------------------------*/
err_t AudioClass::writeWavHeader(File& myFile)
{
  myFile.seek(0);

  m_wav_format.total_size = m_es_size + sizeof(WavaFormat_t) - 8;
  m_wav_format.data_size  = m_es_size;

  int ret = myFile.write((uint8_t*)&m_wav_format, sizeof(WavaFormat_t));
  if (ret < 0)
    {
      print_err("Fail to write file(wav header)\n");
      return AUDIOLIB_ECODE_FILEACCESS_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
#if 0
err_t AudioClass::readFrames(int fd)
{
  size_t data_size = CMN_SimpleFifoGetOccupiedSize(&m_recorder_simple_fifo_handle);

  while (data_size > 0)
    {
      int size = (data_size > FIFO_FRAME_SIZE) ? FIFO_FRAME_SIZE : data_size;

      /* TODO assert で良いよね…。*/
      if (CMN_SimpleFifoPoll(&m_recorder_simple_fifo_handle, (void*)m_es_recorder_buf, size) == 0)
        {
          print_err("ERROR: Fail to get data from simple FIFO.\n");
          return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
        }

      int ret = fwrite(&m_es_recorder_buf, 1, size, fd);
      m_es_size += ret;
      deta_size -= size;

      if (ret < 0)
        {
          print_err("ERROR: Cannot write recorded data to output file.\n");
          fclose(fd);
          return AUDIOLIB_ECODE_FILEACCESS_ERROR;
        }
    }

  return 0;
}
#endif
/*--------------------------------------------------------------------------*/
err_t AudioClass::readFrames(File& myFile)
{
  size_t data_size = CMN_SimpleFifoGetOccupiedSize(&m_recorder_simple_fifo_handle);
  print_dbg("dsize = %d\n", data_size);

  while (data_size > 0)
    {
      int size = (data_size > FIFO_FRAME_SIZE) ? FIFO_FRAME_SIZE : data_size;

      /* TODO: assert で良いよね…。*/
      if (CMN_SimpleFifoPoll(&m_recorder_simple_fifo_handle, (void*)m_es_recorder_buf, size) == 0)
        {
          print_err("ERROR: Fail to get data from simple FIFO.\n");
          return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
        }

      int ret = myFile.write((uint8_t*)&m_es_recorder_buf, size);
      m_es_size += size;
      data_size -= size;

      if (ret < 0)
        {
          print_err("ERROR: Cannot write recorded data to output file.\n");
          myFile.close();
          return AUDIOLIB_ECODE_FILEACCESS_ERROR;
        }
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::readFrames(char* p_buffer, uint32_t buffer_size, uint32_t* read_size)
{
  err_t rst = AUDIOLIB_ECODE_OK;
  if (p_buffer == NULL)
    {
      print_err("ERROR: Buffer area not specified.\n");
      return AUDIOLIB_ECODE_BUFFER_AREA_ERROR;
    }
  if (buffer_size == 0)
    {
      print_err("ERROR: Buffer area size error.\n");
      return AUDIOLIB_ECODE_BUFFER_SIZE_ERROR;
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
          rst = AUDIOLIB_ECODE_INSUFFICIENT_BUFFER_AREA;
        }
      else
        {
          poll_size = data_size;
        }

      if (CMN_SimpleFifoPoll(&m_recorder_simple_fifo_handle, (void*)p_buffer, poll_size) == 0)
        {
          print_err("ERROR: Fail to get data from simple FIFO.\n");
          return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
        }
      *read_size = (uint32_t)poll_size;

      m_es_size += data_size;
    }

  return rst;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::setRenderingClockMode(AsClkMode mode)
{
  AudioCommand command;

  command.header.packet_length = LENGTH_SETRENDERINGCLK;
  command.header.command_code  = AUDCMD_SETRENDERINGCLK;
  command.header.sub_code      = 0x00;
  command.set_renderingclk_param.clk_mode = mode;

  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_SETRENDERINGCLKCMPLT)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
              command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/****************************************************************************
 * Private API on Audio Player
 ****************************************************************************/
extern "C" {
/*--------------------------------------------------------------------------*/
void  input_device_callback(uint32_t size)
{
    /* do nothing */
}
/*--------------------------------------------------------------------------*/
void  output_device_callback(uint32_t size)
{
    /* do nothing */
}

}

/*--------------------------------------------------------------------------*/
err_t AudioClass::set_output(int device)
{
  AudioCommand command;

  command.header.packet_length = LENGTH_INITOUTPUTSELECT;
  command.header.command_code  = AUDCMD_INITOUTPUTSELECT;
  command.header.sub_code      = 0;

  /* Tentative processing until sdk side is modified.
   * Device type is unified by modifying sdk.
   */

  if (device == AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT)
    {
      device = AS_OUT_I2S;
    }
  else
    {
      device = AS_OUT_SP;
    }
  command.init_output_select_param.output_device_sel = device;

  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_INITOUTPUTSELECTCMPLT)
    {
      sleep(1);
      print_err("ERROR: Command (%x) fails. Result code(%x), subcode = %x\n", command.header.command_code, result.header.result_code,result.error_response_param.error_code);

      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::write_fifo(int fd, char *buf, uint32_t write_size, CMN_SimpleFifoHandle *handle)
{

  size_t vacant_size = CMN_SimpleFifoGetVacantSize(handle);
  if (vacant_size < write_size)
    {
      return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
    }

  int ret = read(fd, buf, write_size);

  if (ret < 0)
    {
      print_err("Fail to read file. errno:%d\n",get_errno());
      return AUDIOLIB_ECODE_FILEACCESS_ERROR;
    }

       print_dbg("size = %d!\n",ret);

  if (CMN_SimpleFifoOffer(handle, (const void*)(buf), ret) == 0)
    {
      print_err("Simple FIFO is full!\n");
      return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
    }

   if(ret == 0)
    {
      close(fd);
      return AUDIOLIB_ECODE_FILEEND;
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::write_fifo(File& myFile, char *p_es_buf, uint32_t write_size, CMN_SimpleFifoHandle *handle)
{

  size_t vacant_size = CMN_SimpleFifoGetVacantSize(handle);
  if (vacant_size < write_size)
    {
      return AUDIOLIB_ECODE_OK;
    }

  int ret = -1;

  if (myFile.available())
    {
      ret = myFile.read(p_es_buf, write_size);
    }
  else
    {
      ret = 0;
    }

  if (ret < 0)
    {
      print_err("Fail to read file. errno:%d\n",get_errno());
      return AUDIOLIB_ECODE_FILEACCESS_ERROR;
    }

  if(ret == 0)
    {
      myFile.close();
      return AUDIOLIB_ECODE_FILEEND;
    }

  if (CMN_SimpleFifoOffer(handle, (const void*)(p_es_buf), ret) == 0)
    {
      print_err("Simple FIFO is full!\n");
      return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/****************************************************************************
 * Private API on Audio Recorder
 ****************************************************************************/

err_t AudioClass::init_mic_gain(int dev, int gain)
{
  AudioCommand command;

  command.header.packet_length = LENGTH_INITMICGAIN;
  command.header.command_code  = AUDCMD_INITMICGAIN;
  command.header.sub_code      = 0;

  /* devで、アナログ、デジタルの選択。
  現在、未対応。
  各マイクのGainを個別に調整はしない方向。
  */
  command.init_mic_gain_param.mic_gain[0] = gain;
  command.init_mic_gain_param.mic_gain[1] = gain;
  command.init_mic_gain_param.mic_gain[2] = gain;
  command.init_mic_gain_param.mic_gain[3] = gain;
  command.init_mic_gain_param.mic_gain[4] = 0;
  command.init_mic_gain_param.mic_gain[5] = 0;
  command.init_mic_gain_param.mic_gain[6] = 0;
  command.init_mic_gain_param.mic_gain[7] = 0;

  AS_SendAudioCommand( &command );

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_INITMICGAINCMPLT)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
              command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/****************************************************************************
 * Private API for DSP check
 ****************************************************************************/
bool AudioClass::check_decode_dsp(uint8_t codec_type, const char *path)
{
  char fullpath[32];
  
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
bool AudioClass::check_encode_dsp(uint8_t codec_type, const char *path, uint32_t fs)
{
  char fullpath[32];

  switch (codec_type)
    {
      case AS_CODECTYPE_MP3:
        snprintf(fullpath, sizeof(fullpath), "%s/MP3ENC", path);
        break;

      case AS_CODECTYPE_LPCM:
      case AS_CODECTYPE_WAV:
        if (fs == AS_SAMPLINGRATE_48000)
          {
            return true;
          }
        else
          {
            snprintf(fullpath, sizeof(fullpath), "%s/SRC", path);
          }
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

