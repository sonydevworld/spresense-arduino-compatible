/*
 *  SensorClient.cpp - Sensor library for the Spresense SDK
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
#include <SensorClient.h>

/****************************************************************************
 * Constractor for Sensor Class
 ****************************************************************************/



bool SensorClient::begin(int      id,
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

  /* ID range check */

  if (id < SEN_ID_MAX)
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

      return true;
    }
  else
    {
      /* Fatal error occured. */

      printf("Fail ID out of range.\n");
      return false;
    }
}

bool SensorClient::begin(int      id,
                         uint32_t subscriptions,
                         sensor_data_mh_callback_t cb)
{
  m_id                   = id;
  m_rate                 = 0;
  m_sample_watermark_num = 0;
  m_size_per_sample      = 0;

  /* ID range check */

  if (id < SEN_ID_MAX)
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

      return true;
    }
  else
    {
      /* Fatal error occured. */

      printf("Fail ID out of range.\n");
      return false;
    }
}

bool SensorClient::end(void)
{
  sensor_command_release_t rel;

  /* Release sensor ID */

  rel.header.size = 0;
  rel.header.code = ReleaseClient;
  rel.self        = m_id;
  SS_SendSensorRelease(&rel);

  return true;
}

int SensorClient::publish(PoolId    id,
                          FAR void* data,
                          uint32_t  size_per_sample,
                          uint32_t  freq,
                          uint32_t  sample_watermark_num,
                          uint32_t  timestamp)
{
  /* Check argument of */

  assert(data);

  MemMgrLite::MemHandle mh;
  if (ERR_OK != mh.allocSeg(id, size_per_sample * sample_watermark_num))
    {
      /* Fatal error occured. */

      printf("Fail to allocate segment of memory handle.\n");
      return SENSORCLIENT_ECODE_MEMORY_ALLOCATE_ERROR;
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

  return SENSORCLIENT_ECODE_OK;
}

int SensorClient::publish(MemMgrLite::MemHandle& mh,
                          uint32_t               size_per_sample,
                          uint32_t               freq,
                          uint32_t               sample_watermark_num,
                          uint32_t               timestamp)
{
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

  return SENSORCLIENT_ECODE_OK;
}

int SensorClient::publish(FAR void *data,
                          uint32_t  size_per_sample,
                          uint32_t  freq,
                          uint32_t  sample_watermark_num,
                          uint32_t  timestamp)
{
  return publish(S1_SENSOR_DATA_BUF_POOL,
                 data,                 /** Sensor data address. */
                 size_per_sample,
                 freq,
                 sample_watermark_num,
                 timestamp);
}
