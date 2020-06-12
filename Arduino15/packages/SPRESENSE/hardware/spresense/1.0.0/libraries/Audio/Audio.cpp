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

#include <File.h>

//***************************************************************************
// Definitions
//***************************************************************************
// Configuration ************************************************************
// C++ initialization requires CXX initializer support

#include <asmp/mpshm.h>

extern "C" void  input_device_callback(uint32_t);
extern "C" void  output_device_callback(uint32_t);

/* Timeout waiting for command reception. Unit is milliseconds. */

#define RESPONSE_TIMEOUT 10

/****************************************************************************
 * Debug Functions
 ****************************************************************************/

#ifdef BRD_DEBUG
const char* error_msg[] =
{
   " "
  ,"STATE VIOLATION"
  ,"PACKET LENGTH ERROR"
  ,"COMMAND CODE ERROR"
  ,"COMMAND NOT SUPPOT"
  ,"AUDIO POWER ON ERROR"
  ,"AUDIO POWER OFF ERROR"
  ,"DSP LOAD ERROR"
  ,"DSP UNLOAD ERROR"
  ,"DSP VERSION ERROR"
  ,"SET AUDIO DATA PATH ERROR"
  ,"CLEAR AUDIO DATA PATH ERROR"
  ,"NOT AUDIO DATA PATH"
  ,"DECODER LIB INITIALIZE ERROR"
  ,"ENCODER LIB INITIALIZE ERROR"
  ,"FILTER LIB INITIALIZE ERROR"
  ," "
  ,"COMMAND PARAM CODEC TYPE"
  ," "
  ,"COMMAND PARAM CHANNEL NUMBER"
  ,"COMMAND PARAM SAMPLING RATE"
  ,"COMMAND PARAM BIT RATE"
  ,"COMMAND PARAM BIT LENGTH"
  ,"COMMAND PARAM COMPLEXITY"
  ,"COMMAND PARAM ACTIVE PLAYER"
  ,"COMMAND PARAM INPUT DEVICE"
  ,"COMMAND PARAM OUTPUT DEVICE"
  ,"COMMAND PARAM INPUT HANDLER"
  ,"COMMAND PARAM CONFIG TABLE"
  ,"COMMAND PARAM WITH MFE"
  ,"COMMAND PARAM WITH MPP"
  ," "
  ," "
  ," "
  ," "
  ," "
  ," "
  ,"COMMAND PARAM INPUT DB "
  ," "
  ," "
  ,"DMAC INITIALIZE ERROR "
  ,"DMAC READ ERROR"
  ,"CHECK MEMORY POOL ERROR"
  ,"SIMPLE FIFO UNDERFLOW "
  ,"SET MIC GAIN ERROR"
  ,"SET OUTPUT SELECT ERROR"
  ,"INIT CLEAR STEREO ERROR"
  ,"SET VOLUME ERROR"
  ,"SET VOLUME MUTE ERROR"
  ,"SET BEEP ERROR"
  ,"QUEUE OPERATION ERROR"
  ,"COMMAND PARAM RENDERINGCLK"
};
#endif

#ifdef BRD_DEBUG
const char* attention_code_msg[] =
{
   "INFORMATION"
  ,"WARNING"
  ,"ERROR"
  ,"FATAL"
};

const char* attention_sub_code_msg[] =
{
  " "
  ,"DMA underflow due to transfer queue empty"
  ,"DMA overflow due to capture queue full"
  ,"DMA error from H/W"
  ,"APU queue full for DSP response delay"
  ,"SimpleFIFO underflow"
  ,"SimpleFIFO overflow"
  ,"Illegal request for unacceptable state"
  ,"Internal state error"
  ,"Unexpected parameter"
  ,"Internal queue pop error"
  ,"Internal queue push error"
  ,"Internal queue missing, queue became empty unexpectedly"
  ,"Memory handle alloc error"
  ,"Memory handle free error"
  ,"Task create error"
  ,"Instance resource error"
  ,"Decoded PCM size is 0, ES data may be broken"
  ,"DSP load error"
  ,"DSP unload error"
  ,"DSP execution error due to format error"
  ,"DSP result error"
  ,"DSP illegal reply, Command from DSP may be broken"
  ,"DSP unload done notification"
  ,"Loaded DSP binary version is differ"
  ,"Baseband error, power may be off"
  ,"Stream parse error, initialize parameters may be differ"
  ,"DSP binary load done"
  ,"Recording start"
  ,"Recording stop"
  ,"DSP debug dump log alloc error"
  ,"DSP internal error occured and cannot keep processing"
  ,"DSP send fail"
  ,"Allocate memory of heap area"
};
#endif

/****************************************************************************
 * Common API on Audio Class
 ****************************************************************************/
err_t AudioClass::begin(void)
{
  return begin(NULL);
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::begin(AudioAttentionCb attcb)
{
  int ret;

  m_attention_callback = attcb;

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

static void attentionCallback(const ErrorAttentionParam *attparam)
{
#ifndef BRD_DEBUG
  print_err("Attention!! Level 0x%x Code 0x%x\n", attparam->error_code, attparam->error_att_sub_code);
#else
  print_err("Attention!! Level 0x%x: %s Code 0x%x: %s\n",
    attparam->error_code,         attention_code_msg[attparam->error_code],             
    attparam->error_att_sub_code, attention_sub_code_msg[attparam->error_att_sub_code]);
#endif
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
  AsCreatePlayerParams_t player_create_param;
  player_create_param.msgq_id.player   = MSGQ_AUD_PLY;
  player_create_param.msgq_id.mng      = MSGQ_AUD_MGR;
  player_create_param.msgq_id.mixer    = MSGQ_AUD_OUTPUT_MIX;
  player_create_param.msgq_id.dsp      = MSGQ_AUD_DSP;
  player_create_param.pool_id.es       = S0_DEC_ES_MAIN_BUF_POOL;
  player_create_param.pool_id.pcm      = S0_REND_PCM_BUF_POOL;
  player_create_param.pool_id.dsp      = S0_DEC_APU_CMD_POOL;
  player_create_param.pool_id.src_work = S0_SRC_WORK_MAIN_BUF_POOL;

  int act_rst = AS_CreatePlayerMulti(AS_PLAYER_ID_0, &player_create_param, NULL);
  if (!act_rst)
    {
      print_err("AS_CreatePlayer failed. system memory insufficient!\n");
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  player_create_param.msgq_id.player   = MSGQ_AUD_SUB_PLY;
  player_create_param.msgq_id.mng      = MSGQ_AUD_MGR;
  player_create_param.msgq_id.mixer    = MSGQ_AUD_OUTPUT_MIX;
  player_create_param.msgq_id.dsp      = MSGQ_AUD_SUB_DSP;
  player_create_param.pool_id.es       = S0_DEC_ES_SUB_BUF_POOL;
  player_create_param.pool_id.pcm      = S0_REND_PCM_SUB_BUF_POOL;
  player_create_param.pool_id.dsp      = S0_DEC_APU_CMD_POOL;
  player_create_param.pool_id.src_work = S0_SRC_WORK_SUB_BUF_POOL;

  act_rst = AS_CreatePlayerMulti(AS_PLAYER_ID_1, &player_create_param, NULL);
  if (!act_rst)
    {
      print_err("AS_CreatePlayer failed. system memory insufficient!\n");
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  AsCreateOutputMixParams_t output_mix_create_param;

  output_mix_create_param.msgq_id.mixer = MSGQ_AUD_OUTPUT_MIX;
  output_mix_create_param.msgq_id.render_path0_filter_dsp = MSGQ_AUD_PFDSP0;
  output_mix_create_param.msgq_id.render_path1_filter_dsp = MSGQ_AUD_PFDSP1;
  output_mix_create_param.pool_id.render_path0_filter_pcm = S0_PF0_PCM_BUF_POOL;
  output_mix_create_param.pool_id.render_path1_filter_pcm = S0_PF1_PCM_BUF_POOL;
  output_mix_create_param.pool_id.render_path0_filter_dsp = S0_PF0_APU_CMD_POOL;
  output_mix_create_param.pool_id.render_path1_filter_dsp = S0_PF1_APU_CMD_POOL;

  act_rst = AS_CreateOutputMixer(&output_mix_create_param, NULL);
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
  /* Create MicFrontend. */

  AsCreateMicFrontendParams_t frontend_create_param;
  frontend_create_param.msgq_id.micfrontend = MSGQ_AUD_FRONTEND;
  frontend_create_param.msgq_id.mng         = MSGQ_AUD_MGR;
  frontend_create_param.msgq_id.dsp         = MSGQ_AUD_PREDSP;
  frontend_create_param.pool_id.input       = S0_MIC_IN_BUF_POOL;
  frontend_create_param.pool_id.output      = S0_NULL_POOL;
  frontend_create_param.pool_id.dsp         = S0_PRE_APU_CMD_POOL;

  AS_CreateMicFrontend(&frontend_create_param, NULL);

  /* Create MediaRecorder */

  AsCreateRecorderParams_t recorder_act_param;
  recorder_act_param.msgq_id.recorder      = MSGQ_AUD_RECORDER;
  recorder_act_param.msgq_id.mng           = MSGQ_AUD_MGR;
  recorder_act_param.msgq_id.dsp           = MSGQ_AUD_DSP;
  recorder_act_param.pool_id.input         = S0_MIC_IN_BUF_POOL;
  recorder_act_param.pool_id.output        = S0_OUTPUT_BUF_POOL;
  recorder_act_param.pool_id.dsp           = S0_ENC_APU_CMD_POOL;

  if (!AS_CreateMediaRecorder(&recorder_act_param, NULL))
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

  finalizeMemoryPools();

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
  AS_DeleteMicFrontend();
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
  ids.micfrontend = MSGQ_AUD_FRONTEND;
  ids.mixer       = MSGQ_AUD_OUTPUT_MIX;
  ids.recorder    = MSGQ_AUD_RECORDER;
  ids.effector    = 0xFF;
  ids.recognizer  = 0xFF;

  AS_CreateAudioManager(ids, (m_attention_callback) ? m_attention_callback : attentionCallback);

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
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
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
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::setReadyMode(void)
{
  board_external_amp_mute_control(true);

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
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  free(m_player0_simple_fifo_buf);
  free(m_player1_simple_fifo_buf);

  m_player0_simple_fifo_buf = NULL;
  m_player1_simple_fifo_buf = NULL;

  destroyStaticPools();

  return AUDIOLIB_ECODE_OK;
}
/****************************************************************************
 * Player API on Audio Class
 ****************************************************************************/
err_t AudioClass::setPlayerMode(uint8_t device)
{
  return setPlayerMode(device, AS_SP_DRV_MODE_LINEOUT, SIMPLE_FIFO_BUF_SIZE, WRITE_BUF_SIZE);
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::setPlayerMode(uint8_t device, uint32_t player0bufsize, uint32_t player1bufsize)
{
  return setPlayerMode(device, AS_SP_DRV_MODE_LINEOUT, player0bufsize, player1bufsize);
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::setPlayerMode(uint8_t device, uint8_t sp_drv)
{
  return setPlayerMode(device, sp_drv, SIMPLE_FIFO_BUF_SIZE, WRITE_BUF_SIZE);
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::setPlayerMode(uint8_t device, uint8_t sp_drv, uint32_t player0bufsize, uint32_t player1bufsize)
{
  const NumLayout layout_no = MEM_LAYOUT_PLAYER;

  assert(layout_no < NUM_MEM_LAYOUTS);
  createStaticPools(layout_no);

  AudioClass::set_output(device, sp_drv);

  print_dbg("set output cmplt\n");

  /* Allocate ES buffer */

  if (player0bufsize)
    {
      m_player0_simple_fifo_buf = static_cast<uint32_t *>(malloc(player0bufsize));

      if (m_player0_simple_fifo_buf)
        {
          if (CMN_SimpleFifoInitialize(&m_player0_simple_fifo_handle,
                                       m_player0_simple_fifo_buf,
                                       player0bufsize,
                                       NULL) != 0)
            {
              print_err("Fail to initialize simple FIFO.\n");
              return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
            }

          CMN_SimpleFifoClear(&m_player0_simple_fifo_handle);
        }
    }

  if (player1bufsize)
    {
      m_player1_simple_fifo_buf = static_cast<uint32_t *>(malloc(player1bufsize));

      if (m_player1_simple_fifo_buf)
        {
          if (CMN_SimpleFifoInitialize(&m_player1_simple_fifo_handle,
                                       m_player1_simple_fifo_buf,
                                       player1bufsize,
                                       NULL) != 0)
            {
              print_err("Fail to initialize simple FIFO.\n");
              return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
            }

          CMN_SimpleFifoClear(&m_player1_simple_fifo_handle);
        }
    }

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
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
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

  while (AS_ERR_CODE_OK != AS_ReceiveAudioResult(&result, command.player.player_id, RESPONSE_TIMEOUT));

  if (result.header.result_code != AUDRLT_INITPLAYERCMPLT)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
              command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
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

  while (AS_ERR_CODE_OK != AS_ReceiveAudioResult(&result, command.player.player_id, RESPONSE_TIMEOUT));

  if (result.header.result_code != AUDRLT_PLAYCMPLT)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x) Error subcode(0x%x)\n",
              command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code, result.error_response_param.error_sub_code);
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
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
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);

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

  while (AS_ERR_CODE_OK != AS_ReceiveAudioResult(&result, command.player.player_id, RESPONSE_TIMEOUT));

  if (result.header.result_code != AUDRLT_STOPCMPLT)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
              command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
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
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
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
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
             command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
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
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
              command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

#define WRITE_FRAME_NUM 5
/*--------------------------------------------------------------------------*/
err_t AudioClass::writeFrames(PlayerId id, int fd)
{
  int ret = AUDIOLIB_ECODE_OK;

  uint32_t *p_fifo = (id == Player0)
    ? m_player0_simple_fifo_buf : m_player1_simple_fifo_buf;

  if (!p_fifo)
    {
      printf("Buffer is not allocated.\n");
      return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
    }

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

  uint32_t *p_fifo = (id == Player0)
    ? m_player0_simple_fifo_buf : m_player1_simple_fifo_buf;

  if (!p_fifo)
    {
      printf("Buffer is not allocated.\n");
      return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
    }

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

/*--------------------------------------------------------------------------*/
err_t AudioClass::writeFrames(PlayerId id, uint8_t *data, uint32_t write_size)
{
  int ret = AUDIOLIB_ECODE_OK;

  uint32_t *p_fifo = (id == Player0)
    ? m_player0_simple_fifo_buf : m_player1_simple_fifo_buf;

  if (!p_fifo)
    {
      printf("Buffer is not allocated.\n");
      return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
    }

  CMN_SimpleFifoHandle *handle =
    (id == Player0) ?
      &m_player0_simple_fifo_handle : &m_player1_simple_fifo_handle;

  size_t vacant_size = CMN_SimpleFifoGetVacantSize(handle);

  if (write_size <= 0)
    {
      return AUDIOLIB_ECODE_OK;
    }

  if (vacant_size < write_size)
    {
      return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
    }

  if (CMN_SimpleFifoOffer(handle, (const void*)(data), write_size) == 0)
    {
      print_err("Simple FIFO is full!\n");
      return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
    }

  return ret;
}

/****************************************************************************
 * Recoder API on Audio Class
 ****************************************************************************/
#define m_recorder_simple_fifo_handle m_player0_simple_fifo_handle
#define m_recorder_simple_fifo_buf m_player0_simple_fifo_buf
#define m_es_recorder_buf m_es_player0_buf

err_t AudioClass::setRecorderMode(uint8_t input_device, int32_t input_gain)
{
  return setRecorderMode(input_device, input_gain, SIMPLE_FIFO_BUF_SIZE, false);
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::setRecorderMode(uint8_t input_device, int32_t input_gain, uint32_t bufsize, bool is_digital)
{
  const NumLayout layout_no = MEM_LAYOUT_RECORDER;

  assert(layout_no < NUM_MEM_LAYOUTS);
  createStaticPools(layout_no);

  if (!bufsize)
    {
      print_err("Invalid buffer size.\n");
      return AUDIOLIB_ECODE_BUFFER_SIZE_ERROR;
    }

  m_recorder_simple_fifo_buf = static_cast<uint32_t *>(malloc(bufsize));

  if (!m_recorder_simple_fifo_buf)
    {
      print_err("Fail to allocate memory.\n");
      return AUDIOLIB_ECODE_BUFFER_AREA_ERROR;
    }

  if (CMN_SimpleFifoInitialize(&m_recorder_simple_fifo_handle, m_recorder_simple_fifo_buf, bufsize, NULL) != 0)
    {
      print_err("Fail to initialize simple FIFO.\n");
      return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
    }

  CMN_SimpleFifoClear(&m_recorder_simple_fifo_handle);

  m_output_device_handler.simple_fifo_handler = (void*)(&m_recorder_simple_fifo_handle);
  m_output_device_handler.callback_function = output_device_callback;

  if ((input_device == AS_SETRECDR_STS_INPUTDEVICE_MIC) && is_digital)
    {
      uint8_t dig_map[] = { 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc };

      if (set_mic_map(dig_map) != AUDIOLIB_ECODE_OK)
        {
          print_err("Set mic mapping error!\n");
        }
    }

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
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  if (init_mic_gain(input_device, input_gain) != AUDIOLIB_ECODE_OK)
    {
      print_err("Mic init error!");
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::setRecorderMode(uint8_t input_device, int32_t input_gain, uint32_t bufsize)
{
  return setRecorderMode(input_device, input_gain, bufsize, false);
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::setRecorderMode(uint8_t input_device)
{
  return setRecorderMode(input_device, 0, SIMPLE_FIFO_BUF_SIZE, false);
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::initMicFrontend(uint8_t ch_num, uint8_t bit_length, uint16_t samples)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_INIT_MICFRONTEND;
  command.header.command_code  = AUDCMD_INIT_MICFRONTEND;
  command.header.sub_code      = 0x00;
  command.init_micfrontend_param.ch_num       = ch_num;
  command.init_micfrontend_param.bit_length   = bit_length;
  command.init_micfrontend_param.samples      = samples;
  command.init_micfrontend_param.preproc_type = AsMicFrontendPreProcThrough;
  snprintf(command.init_micfrontend_param.preprocess_dsp_path,
           AS_PREPROCESS_FILE_PATH_LEN,
           "\0");
  command.init_micfrontend_param.data_dest = AsMicFrontendDataToRecorder;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  if (result.header.result_code != AUDRLT_INIT_MICFRONTEND)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
                command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
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
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  m_wav_format.riff     = CHUNKID_RIFF;
  m_wav_format.wave     = FORMAT_WAVE;
  m_wav_format.fmt      = SUBCHUNKID_FMT;
  m_wav_format.fmt_size = FMT_CHUNK_SIZE;
  m_wav_format.format   = FORMAT_ID_PCM;
  m_wav_format.channel  = channel_number;
  m_wav_format.rate     = sampling_rate;
  m_wav_format.avgbyte  = sampling_rate * channel_number * (bit_length / 8);
  m_wav_format.block    = channel_number * (bit_length / 8);
  m_wav_format.bit      = bit_length;
  m_wav_format.data     = SUBCHUNKID_DATA;

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
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
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
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
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
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
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

  initMicFrontend(channel, bit_length, getCapSampleNumPerFrame(codec_type, sampling_rate));

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
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
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
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

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
err_t AudioClass::writeWavHeader(File& myFile)
{
  myFile.seek(0);

  m_wav_format.total_size = m_es_size + sizeof(WAVHEADER) - 8;
  m_wav_format.data_size  = m_es_size;

  int ret = myFile.write((uint8_t*)&m_wav_format, sizeof(WAVHEADER));
  if (ret != sizeof(WAVHEADER))
    {
      print_err("Fail to write file(wav header)\n");
      return AUDIOLIB_ECODE_FILEACCESS_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::readFrames(File& myFile)
{
  if (!m_recorder_simple_fifo_buf)
    {
      print_err("ERROR: FIFO area is not allocated.\n");
      return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
    }

  size_t data_size = CMN_SimpleFifoGetOccupiedSize(&m_recorder_simple_fifo_handle);
  print_dbg("dsize = %d\n", data_size);

  while (data_size > 0)
    {
      int size = (data_size > FIFO_FRAME_SIZE) ? FIFO_FRAME_SIZE : data_size;

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

  if (!m_recorder_simple_fifo_buf)
    {
      print_err("ERROR: FIFO area is not allocated.\n");
      return AUDIOLIB_ECODE_SIMPLEFIFO_ERROR;
    }

  size_t data_size = CMN_SimpleFifoGetOccupiedSize(&m_recorder_simple_fifo_handle);
  print_dbg("dsize = %d\n", data_size);

  *read_size = 0;
  size_t poll_size = 0;
  if (data_size > 0)
    {
      if (data_size > buffer_size)
        {
          print_dbg("WARNING: Insufficient buffer area.\n");
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
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/****************************************************************************
 * Baseband through API on Audio Class
 ****************************************************************************/
err_t AudioClass::setThroughMode(ThroughInput input, ThroughI2sOut i2s_out, bool sp_out, int32_t input_gain, uint8_t sp_drv)
{
  if (i2s_out == None)
    {
      AudioClass::set_output(AS_SETPLAYER_OUTPUTDEVICE_SPHP, sp_drv);
    }
  else
    {
      AudioClass::set_output(AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT, sp_drv);
    }

  AudioCommand command;
  command.header.packet_length = LENGTH_SET_THROUGH_PATH;
  command.header.command_code  = AUDCMD_SETTHROUGHPATH;
  command.header.sub_code      = 0x00;

  switch (input)
    {
      case MicIn:
        init_mic_gain(AS_SETRECDR_STS_INPUTDEVICE_MIC,input_gain);
        send_set_through();

        command.set_through_path.path1.en  = true;
        command.set_through_path.path1.in  = AS_THROUGH_PATH_IN_MIC;
        command.set_through_path.path2.en  = false;
        break;

      case I2sIn:
        send_set_through();

        command.set_through_path.path1.en  = false;
        command.set_through_path.path2.en  = true;
        command.set_through_path.path2.in  = AS_THROUGH_PATH_IN_I2S1;
        command.set_through_path.path2.out = AS_THROUGH_PATH_OUT_MIXER1;
        break;

      case BothIn:
        init_mic_gain(AS_SETRECDR_STS_INPUTDEVICE_MIC,input_gain);
        send_set_through();

        command.set_through_path.path1.en  = true;
        command.set_through_path.path1.in  = AS_THROUGH_PATH_IN_MIC;
        command.set_through_path.path2.en  = true;
        command.set_through_path.path2.in  = AS_THROUGH_PATH_IN_I2S1;
        command.set_through_path.path2.out = AS_THROUGH_PATH_OUT_MIXER1;
        break;

      default:
        return AUDIOLIB_ECODE_PARAMETER_ERROR; /* error. tentative. */
    }

  if (i2s_out == Mic)
    {
      command.set_through_path.path1.out = AS_THROUGH_PATH_OUT_I2S1;
    }
  else
    {
      command.set_through_path.path1.out = AS_THROUGH_PATH_OUT_MIXER1;
    }

  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_SETTHROUGHPATHCMPLT)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
                command.header.command_code,
                result.header.result_code,
                result.error_response_param.module_id,
                result.error_response_param.error_code);
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);

      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  if (i2s_out == Mixer)
    {
      command.set_through_path.path1.en  = true;
      command.set_through_path.path1.in  = AS_THROUGH_PATH_IN_MIXER;
      command.set_through_path.path1.out = AS_THROUGH_PATH_OUT_I2S1;
      command.set_through_path.path2.en  = false;

      AS_SendAudioCommand(&command);

      AS_ReceiveAudioResult(&result);

      if (result.header.result_code != AUDRLT_SETTHROUGHPATHCMPLT)
        {
          print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
                    command.header.command_code,
                    result.header.result_code,
                    result.error_response_param.module_id,
                    result.error_response_param.error_code);
          print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);

          return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
        }
    }

  if (sp_out)
    {
      board_external_amp_mute_control(false);
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
err_t AudioClass::set_output(uint8_t device, uint8_t sp_drv)
{
  AudioCommand command;

  /* Set output device. */

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
      print_err("ERROR: Command (%x) fails. Result code(%x), subcode = %x\n", command.header.command_code, result.header.result_code,result.error_response_param.error_code);
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  /* Set speaker driver mode. */

  command.header.packet_length = LENGTH_SETSPDRVMODE;
  command.header.command_code  = AUDCMD_SETSPDRVMODE;
  command.header.sub_code      = 0;
  command.set_sp_drv_mode.mode = sp_drv;

  AS_SendAudioCommand(&command);

  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_SETSPDRVMODECMPLT)
    {
      print_err("ERROR: Command (%x) fails. Result code(%x), subcode = %x\n", command.header.command_code, result.header.result_code,result.error_response_param.error_code);
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
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

err_t AudioClass::set_mic_map(uint8_t map[AS_MIC_CHANNEL_MAX])
{
  AudioCommand command;

  command.header.packet_length = LENGTH_SETMICMAP;
  command.header.command_code  = AUDCMD_SETMICMAP;
  command.header.sub_code      = 0;

  memcpy(command.set_mic_map_param.mic_map, map, sizeof(command.set_mic_map_param.mic_map));

  AS_SendAudioCommand( &command );

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_SETMICMAPCMPLT)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
                command.header.command_code,
                result.header.result_code,
                result.error_response_param.module_id,
                result.error_response_param.error_code);
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t AudioClass::init_mic_gain(int dev, int gain)
{
  AudioCommand command;

  command.header.packet_length = LENGTH_INITMICGAIN;
  command.header.command_code  = AUDCMD_INITMICGAIN;
  command.header.sub_code      = 0;

  /* ”dev” can select  analog or digital, but currently not supported. 
     It's not  suport that the gain of each microphone adjust individually.
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
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/****************************************************************************
 * Private API on Audio BaseBand
 ****************************************************************************/
err_t AudioClass::send_set_through(void)
{
  AudioCommand command;

  command.header.packet_length = LENGTH_SET_THROUGH_STATUS;
  command.header.command_code = AUDCMD_SETTHROUGHSTATUS;
  command.header.sub_code = 0x00;

  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);

  if (result.header.result_code != AUDRLT_STATUSCHANGED)
    {
      print_err("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
              command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
      print_dbg("ERROR: %s\n", error_msg[result.error_response_param.error_code]);
      return AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR;
    }

  return AUDIOLIB_ECODE_OK;
}

/****************************************************************************
 * Private API for DSP check
 ****************************************************************************/
bool AudioClass::check_decode_dsp(uint8_t codec_type, const char *path)
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

/*--------------------------------------------------------------------------*/
bool AudioClass::check_encode_dsp(uint8_t codec_type, const char *path, uint32_t fs)
{
  char fullpath[32] = { 0 };
  struct stat buf;
  int retry;
  int ret = 0;

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

