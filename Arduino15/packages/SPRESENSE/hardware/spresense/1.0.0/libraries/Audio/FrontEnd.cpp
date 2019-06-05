/*
 *  FrontEnd.cpp - FrontEnd implement file for the Spresense SDK
 *  Copyright 2019 Sony Semiconductor Solutions Corporation
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

#include "FrontEnd.h"

#include "memutil/msgq_id.h"
#include "memutil/mem_layout.h"
#include "memutil/memory_layout.h"

/*--------------------------------------------------------------------------*/
extern "C" {

static void attentionCallback(const ErrorAttentionParam *attparam)
{
  print_err("Attention!! Level 0x%x Code 0x%x\n", attparam->error_code, attparam->error_att_sub_code);
}

}

/****************************************************************************
 * Public API on FrontEnd Class
 ****************************************************************************/

err_t FrontEnd::begin(void)
{
  return begin(NULL);
}

/*--------------------------------------------------------------------------*/
err_t FrontEnd::begin(AudioAttentionCb attcb)
{
  bool result;

  /* Create Frontend. */

  AsCreateMicFrontendParam_t frontend_create_param;
  frontend_create_param.msgq_id.micfrontend = MSGQ_AUD_FRONTEND;
  frontend_create_param.msgq_id.mng         = MSGQ_AUD_MGR;
  frontend_create_param.msgq_id.dsp         = MSGQ_AUD_PREDSP;
  frontend_create_param.pool_id.input       = MIC_IN_BUF_POOL;
  frontend_create_param.pool_id.output      = NULL_POOL;
  frontend_create_param.pool_id.dsp         = PRE_APU_CMD_POOL;

  result = AS_CreateMicFrontend(&frontend_create_param, (attcb) ? attcb : attentionCallback);
  if (!result)
    {
      print_err("Error: AS_CreateFrontend() failure!\n");
      return FRONTEND_ECODE_COMMAND_ERROR;
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
      return FRONTEND_ECODE_COMMAND_ERROR;
    }

  return FRONTEND_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t FrontEnd::end(void)
{
  bool result;

  /* Delete Frontend */

  result = AS_DeleteMicFrontend();
  if (!result)
    {
      print_err("Error: AS_DeleteFrontend() failure!\n");
      return FRONTEND_ECODE_COMMAND_ERROR;
    }

  /* Delete Capture */

  result = AS_DeleteCapture();
  if (!result)
    {
      print_err("Error: AS_DeleteCapture() failure!\n");
      return FRONTEND_ECODE_COMMAND_ERROR;
    }

  return FRONTEND_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t FrontEnd::activate(AsMicFrontendPreProcType proc_type)
{
  return activate(proc_type, NULL);
}

/*--------------------------------------------------------------------------*/
err_t FrontEnd::activate(AsMicFrontendPreProcType proc_type, MicFrontendCallback fedcb)
{
  bool result;

  /* Activate Frontend */

  AsActivateMicFrontend frontend_act;

  frontend_act.param.input_device = AsMicFrontendDeviceMic;
  frontend_act.param.preproc_type = proc_type;
  frontend_act.cb                 = fedcb;

  m_fed_callback = fedcb;

  result = AS_ActivateMicFrontend(&frontend_act);
  if (!result)
    {
      print_err("Error: AS_ActivateMicFrontend() failure!\n");
      return FRONTEND_ECODE_COMMAND_ERROR;
    }

  if (!m_fed_callback)
    {
      AudioObjReply reply_info;
      result = AS_ReceiveObjectReply(MSGQ_AUD_MGR, &reply_info);
      if (!result)
        {
          print_err("Error: AS_ReceiveObjectReply() failure!\n");
          return FRONTEND_ECODE_COMMAND_ERROR;
        }
    }

  /* Activate Baseband */

  err_t bb_result = activateBaseband();
  if (bb_result != FRONTEND_ECODE_OK)
    {
      print_err("Error: Baseband activation() failure!\n");
      return bb_result;
    }

  return FRONTEND_ECODE_OK;
}
 
/*--------------------------------------------------------------------------*/
err_t FrontEnd::init(uint8_t channel_number,
                     uint8_t bit_length,
                     uint32_t samples_per_frame,
                     uint8_t data_path,
                     AsDataDest dest)
{
  bool result;

  /* Init Frontend */

  AsInitMicFrontendParam frontend_init;

  frontend_init.channel_number    = channel_number;
  frontend_init.bit_length        = bit_length;
  frontend_init.samples_per_frame = samples_per_frame;
  frontend_init.data_path         = data_path;
  frontend_init.dest              = dest;

  result = AS_InitMicFrontend(&frontend_init);
  if (!result)
    {
      print_err("Error: AS_InitFrontend() failure!\n");
      return FRONTEND_ECODE_COMMAND_ERROR;
    }

  if (!m_fed_callback)
    {
      AudioObjReply reply_info;
      result = AS_ReceiveObjectReply(MSGQ_AUD_MGR, &reply_info);
      if (!result)
        {
          print_err("Error: AS_ReceiveObjectReply() failure!\n");
          return FRONTEND_ECODE_COMMAND_ERROR;
        }
    }

  return FRONTEND_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t FrontEnd::start(void)
{
  bool result;

  /* Start Frontend */

  AsStartMicFrontendParam frontend_start;
  result = AS_StartMicFrontend(&frontend_start);
  if (!result)
    {
      print_err("Error: AS_StartFrontend() failure!\n");
      return FRONTEND_ECODE_COMMAND_ERROR;
    }

  if (!m_fed_callback)
    {
      AudioObjReply reply_info;
      result = AS_ReceiveObjectReply(MSGQ_AUD_MGR, &reply_info);
      if (!result)
        {
          print_err("Error: AS_ReceiveObjectReply() failure!\n");
          return FRONTEND_ECODE_COMMAND_ERROR;
        }
    }
 
  return FRONTEND_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t FrontEnd::stop(void)
{
  bool result;

  /* Stop Frontend */

  AsStopMicFrontendParam frontend_stop;
  result = AS_StopMicFrontend(&frontend_stop);
  if (!result)
    {
      print_err("Error: AS_StopFrontend() failure!\n");
      return FRONTEND_ECODE_COMMAND_ERROR;
    }

  if (!m_fed_callback)
    {
      AudioObjReply reply_info;
      result = AS_ReceiveObjectReply(MSGQ_AUD_MGR, &reply_info);
      if (!result)
        {
          print_err("Error: AS_ReceiveObjectReply() failure!\n");
          return FRONTEND_ECODE_COMMAND_ERROR;
        }
    }

  return FRONTEND_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t FrontEnd::initpreproc(AsInitPreProcParam *param)
{
  bool result;

  result = AS_InitPreprocFrontend(param);
  if (!result)
    {
      print_err("Error: AS_StopFrontend() failure!\n");
      return FRONTEND_ECODE_COMMAND_ERROR;
    }

  if (!m_fed_callback)
    {
      AudioObjReply reply_info;
      result = AS_ReceiveObjectReply(MSGQ_AUD_MGR, &reply_info);
      if (!result)
        {
          print_err("Error: AS_ReceiveObjectReply() failure!\n");
          return FRONTEND_ECODE_COMMAND_ERROR;
        }
    }

  return FRONTEND_ECODE_OK;
}
 
/*--------------------------------------------------------------------------*/
err_t FrontEnd::setpreproc(AsInitPreProcParam *param)
{
  bool result;

  result = AS_SetPreprocMicFrontend(param);
  if (!result)
    {
      print_err("Error: AS_StopFrontend() failure!\n");
      return FRONTEND_ECODE_COMMAND_ERROR;
    }

  if (!m_fed_callback)
    {
      AudioObjReply reply_info;
      result = AS_ReceiveObjectReply(MSGQ_AUD_MGR, &reply_info);
      if (!result)
        {
          print_err("Error: AS_ReceiveObjectReply() failure!\n");
          return FRONTEND_ECODE_COMMAND_ERROR;
        }
    }

  return FRONTEND_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t FrontEnd::setMicGain(int16_t mic_gain)
{
  AsMicFrontendMicGainParam micgain_param;

  for (int i = 0; i < AS_MIC_CHANNEL_MAX; i++)
    {
      micgain_param.mic_gain[i] = mic_gain;
    }

  bool result = AS_SetMicGainMicFrontend(&micgain_param);
  if (!result)
    {
      print_err("Error: AS_SetMicGainMediaRecorder() failure!\n");
      return FRONTEND_ECODE_COMMAND_ERROR;
    }

  if (!m_fed_callback)
    {
      AudioObjReply reply_info;
      result = AS_ReceiveObjectReply(MSGQ_AUD_MGR, &reply_info);
      if (!result)
        {
          print_err("Error: AS_ReceiveObjectReply() failure!\n");
          return FRONTEND_ECODE_COMMAND_ERROR;
        }
    }

  return FRONTEND_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t FrontEnd::deactivate(void)
{
  bool result;

  /* Deactivate Frontend */

  AsDeactivateMicFrontendParam frontend_deact;
  result = AS_DeactivateMicFrontend(&frontend_deact);
  if (!result)
    {
      print_err("Error: AS_DeactivateFrontend() failure!\n");
      return FRONTEND_ECODE_COMMAND_ERROR;
    }

  if (!m_fed_callback)
    {
      AudioObjReply reply_info;
      result = AS_ReceiveObjectReply(MSGQ_AUD_MGR, &reply_info);
      if (!result)
        {
          print_err("Error: AS_ReceiveObjectReply() failure!\n");
          return FRONTEND_ECODE_COMMAND_ERROR;
        }
    }

  /* Deactivate baseband */

  err_t bb_result = deactivateBaseband();
  if (bb_result != FRONTEND_ECODE_OK)
    {
      print_err("Error: Baseband deactivateion failure!\n");
      return bb_result;
    }

  return FRONTEND_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t FrontEnd::activateBaseband(void)
{
  CXD56_AUDIO_ECODE error_code;

  /* Power on audio device */

  if (cxd56_audio_get_status() == CXD56_AUDIO_POWER_STATE_OFF)
    {
      error_code = cxd56_audio_poweron();

      if (error_code != CXD56_AUDIO_ECODE_OK)
        {
          print_err("cxd56_audio_poweron() error! [%d]\n", error_code);
          return FRONTEND_ECODE_BASEBAND_ERROR;
        }
    }

  /* Enable input */

  error_code = cxd56_audio_en_input();

  if (error_code != CXD56_AUDIO_ECODE_OK)
    {
      print_err("cxd56_audio_en_input() error! [%d]\n", error_code);
      return FRONTEND_ECODE_BASEBAND_ERROR;
    }

  return FRONTEND_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t FrontEnd::deactivateBaseband(void)
{
  CXD56_AUDIO_ECODE error_code;

  /* Disable input */

  error_code = cxd56_audio_dis_input();

  if (error_code != CXD56_AUDIO_ECODE_OK)
    {
      print_err("cxd56_audio_dis_input() error! [%d]\n", error_code);
      return FRONTEND_ECODE_BASEBAND_ERROR;
    }

  /* Power off audio device */

  if (cxd56_audio_get_status() == CXD56_AUDIO_POWER_STATE_ON)
    {
      error_code = cxd56_audio_poweroff();

      if (error_code != CXD56_AUDIO_ECODE_OK)
        {
          print_err("cxd56_audio_poweroff() error! [%d]\n", error_code);
          return FRONTEND_ECODE_BASEBAND_ERROR;
        }
    }

  return FRONTEND_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t FrontEnd::setCapturingClkMode(uint8_t clk_mode)
{
  CXD56_AUDIO_ECODE error_code = CXD56_AUDIO_ECODE_OK;

  cxd56_audio_clkmode_t mode;

  mode = (clk_mode == FRONTEND_CAPCLK_NORMAL)
           ? CXD56_AUDIO_CLKMODE_NORMAL : CXD56_AUDIO_CLKMODE_HIRES;

  error_code = cxd56_audio_set_clkmode(mode);

  if (error_code != CXD56_AUDIO_ECODE_OK)
    {
      print_err("cxd56_audio_set_clkmode() error! [%d]\n", error_code);
      return FRONTEND_ECODE_BASEBAND_ERROR;
    }

  return FRONTEND_ECODE_OK;
}

