/*
 *  Sensor.cpp - Sensor library for the Spresense SDK
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nuttx/init.h>
#include <nuttx/arch.h>
#include <asmp/mpshm.h>

#include "memutil/msgq_id.h"
#include "memutil/mem_layout.h"

#include "Sensor.h"
#include "MemoryUtil.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/

static step_counter_notify_result s_app_callback;
static FAR StepCounterClass *s_step_counter_ins;

/****************************************************************************
 * Callback function for Sensor Class
 ****************************************************************************/

static void sensor_manager_api_response(unsigned int code,
                                        unsigned int ercd,
                                        unsigned int self)
{
  if (ercd != SS_ECODE_OK)
    {
      print_err("Error: get api response. code %d, ercd %d, self %d\n",
                code, ercd, self);
    }

  return;
}

/*--------------------------------------------------------------------------*/
static bool step_counter_receive_data(sensor_command_data_mh_t& data)
{
  StepCounterWrite(s_step_counter_ins, &data);

  return true;
}

/*--------------------------------------------------------------------------*/
static bool step_counter_receive_result(sensor_command_data_mh_t& data)
{
  FAR SensorCmdStepCounter *result_data =
    reinterpret_cast<SensorCmdStepCounter *>(data.mh.getVa());
  if (SensorOK == result_data->result.exec_result)
    {
      if (result_data->exec_cmd.cmd_type == 
            STEP_COUNTER_CMD_UPDATE_ACCELERATION)
        {
          /* Call application callback */

          (*s_app_callback)(&result_data->result.steps);
        }
    }

  return true;
}

/****************************************************************************
 * Public method on Sensor Class
 ****************************************************************************/

err_t SensorClass::begin(void)
{
  initMemoryPools();
  createStaticPools(0);

  if (!SS_ActivateSensorSubSystem(MSGQ_SEN_MGR, sensor_manager_api_response))
    {
      print_err("Sensor activation error.\n");
      return SENSORLIB_ECODE_ACTIVATE_MANAGER_ERROR;
    }
  return SENSORLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t SensorClass::end(void)
{
  if (!SS_DeactivateSensorSubSystem())
    {
      print_err("Sensor deactivation error.\n");
      return SENSORLIB_ECODE_DEACTIVATE_MANAGER_ERROR;
    }

  destroyStaticPools();  
  return SENSORLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t SensorClass::startStepCounter(step_counter_t &param)
{
  sensor_command_register_t reg;  
  reg.header.size   = 0;
  reg.header.code   = ResisterClient;
  reg.self          = accelID;
  reg.subscriptions = 0; 
  reg.callback      = NULL;
  reg.callback_mh   = NULL;
  SS_SendSensorResister(&reg);

  reg.header.size   = 0;
  reg.header.code   = ResisterClient;
  reg.self          = stepcounterID;
  reg.subscriptions = (0x01 << accelID);
  reg.callback      = NULL;
  reg.callback_mh   = &step_counter_receive_data;
  SS_SendSensorResister(&reg);

  reg.header.size   = 0;
  reg.header.code   = ResisterClient;
  reg.self          = app0ID;
  reg.subscriptions = (0x01 << stepcounterID);
  reg.callback      = NULL;
  reg.callback_mh   = &step_counter_receive_result;
  SS_SendSensorResister(&reg);

  s_step_counter_ins = StepCounterCreate(SENSOR_DSP_CMD_BUF_POOL);
  if (NULL == s_step_counter_ins)
    {
      print_err("Error: StepCounterCreate() failure.\n");
      return EXIT_FAILURE;
    }

  int ret = StepCounterOpen(s_step_counter_ins);
  if (ret != SS_ECODE_OK)
    {
      print_err("Error: StepCounterOpen() failure. error = %d\n", ret);
      return EXIT_FAILURE;
    }

  /* Setup Stride setting.
   * The range of configurable stride lenght is 1 - 249[cm].
   * For the mode, set STEP_COUNTER_MODE_FIXED_LENGTH fixed.
   */

  StepCounterSetting set;
  set.walking.step_length = param.walking_stride;
  set.walking.step_mode   = STEP_COUNTER_MODE_FIXED_LENGTH;
  set.running.step_length = param.running_stride;
  set.running.step_mode   = STEP_COUNTER_MODE_FIXED_LENGTH;
  ret = StepCounterSet(s_step_counter_ins, &set);
  if (ret != SS_ECODE_OK)
    {
      print_err("Error: StepCounterSet() failure. error = %d\n", ret);
      return EXIT_FAILURE;
    }

  s_app_callback = param.callback;

  return SENSORLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t SensorClass::stopStepCounter(void)
{
  if (SS_ECODE_OK != StepCounterClose(s_step_counter_ins))
    {
      return SENSORLIB_ECODE_CLOSE_STEPCOUNTER_ERROR;
    }

  sensor_command_release_t rel;

  rel.header.size = 0;
  rel.header.code = ReleaseClient;
  rel.self        = app0ID;
  SS_SendSensorRelease(&rel);

  rel.header.size = 0;
  rel.header.code = ReleaseClient;
  rel.self        = stepcounterID;
  SS_SendSensorRelease(&rel);

  rel.header.size = 0;
  rel.header.code = ReleaseClient;
  rel.self        = accelID;
  SS_SendSensorRelease(&rel);

  return SENSORLIB_ECODE_OK;
}

/*--------------------------------------------------------------------------*/
err_t SensorClass::write_data(SensorClientID id,
                              FAR accel_float_t *data,
                              uint32_t sample_freq,
                              uint32_t sample_num,
                              uint32_t timestamp)
{
  return write_data<accel_float_t>(id, data, sample_freq, sample_num, timestamp);
}

/****************************************************************************
 * Private method on Sensor Class
 ****************************************************************************/

template <typename T>
err_t SensorClass::write_data(SensorClientID id,
                              FAR T *data,
                              uint32_t sample_freq,
                              uint32_t sample_num,
                              uint32_t timestamp)
{
  /* Check argument of */

  if (data == NULL)
    {
      return SENSORLIB_ECODE_ARGUMENT_NULL_ERROR;
    }

  PoolId pool_id;
  switch (id)
    {
      case accelID:
        pool_id = ACCEL_DATA_BUF_POOL;
        break;

      default:
        /* Not support */

        return SENSORLIB_ECODE_SENSOR_CLIENT_ID_ERROR;
    }

  MemMgrLite::MemHandle mh;
  if (ERR_OK != mh.allocSeg(pool_id,
                            sizeof(T) * sample_num))
    {
      /* Fatal error occured. */

      print_err("Fail to allocate segment of memory handle.\n");
      return SENSORLIB_ECODE_CLOSE_STEPCOUNTER_ERROR;
    }
  FAR char *p_dst = reinterpret_cast<char *>(mh.getPa());

  /* CPU copy from buffer to MemHandle. */

  memcpy(p_dst, (char *)data, sizeof(T) * sample_num);

  /* Send data to logical sensor. */

  sensor_command_data_mh_t packet;
  packet.header.size = 0;
  packet.header.code = SendData;
  packet.self        = id;
  packet.time        = timestamp & 0xFFFFFF; /* 24bits */
  packet.fs          = sample_freq;
  packet.size        = sample_num;
  packet.mh          = mh;
  SS_SendSensorDataMH(&packet);

  return SENSORLIB_ECODE_OK;
}
