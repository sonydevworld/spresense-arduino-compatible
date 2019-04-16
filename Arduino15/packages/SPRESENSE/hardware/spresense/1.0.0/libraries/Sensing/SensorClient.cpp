/*
 *  SensorClient.cpp - SPI implement file for the Spresense SDK
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
#include <SensorClient.h>


/****************************************************************************
 * Private Data
 ****************************************************************************/
static bool isFirst = true;  /* First instance */


/****************************************************************************
 * Callback function for Sensor Class
 ****************************************************************************/
static void sensor_manager_api_response(unsigned int code,
                                        unsigned int ercd,
                                        unsigned int self)
{
  if (ercd != SS_ECODE_OK)
    {
      printf("Error: get api response. code %d, ercd %d, self %d\n",
                code, ercd, self);
    }

  return;
}

SensorClient::SensorClient(int      id,
                           uint32_t subscriptions,
                           int      rate,
                           int      sample_watermark_num,
                           int      size_per_sample,
                           sensor_data_mh_callback_t cb)
{
  m_id                   = id;
  m_rate                 = rate;
  m_sample_watermark_num = sample_watermark_num;
  m_size_per_sample      = size_per_sample;

  if (isFirst)
    {
      initMemoryPools();
      createStaticPools(MEM_LAYOUT_SENSORS);

      /* Create sensor manager pthread */

      SS_ActivateSensorSubSystem(MSGQ_SEN_MGR, sensor_manager_api_response);

      isFirst = false;
    }

  /* ID range check */

  if (id < APP_ID_MAX)
    {
      /* Registed sensor ID */

      sensor_command_register_t reg;

      reg.header.size     = 0;
      reg.header.code     = ResisterClient;
      reg.self            = id;
      reg.subscriptions   = subscriptions; 
      reg.callback        = NULL;
      reg.callback_mh     = cb;
      SS_SendSensorResister(&reg);

    }
  else
    {
      /* Fatal error occured. */

      printf("Fail ID out of range.\n");
    }
}


int SensorClient::publish(PoolId    id,
                          FAR void* data,
                          uint32_t  size_per_sample,
                          uint32_t  freq,
                          uint32_t  sample_watermark_num,
                          uint32_t  timestamp)
{
  /* Check argument of */

  if (data == NULL)
    {
      return ERR_NG;
    }

  MemMgrLite::MemHandle mh;
  if (ERR_OK != mh.allocSeg(id, size_per_sample * sample_watermark_num))
    {
      /* Fatal error occured. */

      printf("Fail to allocate segment of memory handle.\n");
      return ERR_NG;
    }
  FAR char *p_dst = reinterpret_cast<char *>(mh.getPa());

  /* CPU copy from buffer to MemHandle. */

  memcpy(p_dst, (char *)data, size_per_sample * sample_watermark_num);

  /* Send data to logical sensor. */

  sensor_command_data_mh_t packet;
  packet.header.size = 0;
  packet.header.code = SendData;
  packet.self        = m_id;
  packet.time        = timestamp & 0xFFFFFF; /* 24bits */
  packet.fs          = freq;
  packet.size        = sample_watermark_num;
  packet.mh          = mh;
  SS_SendSensorDataMH(&packet);

  return ERR_OK;
}
