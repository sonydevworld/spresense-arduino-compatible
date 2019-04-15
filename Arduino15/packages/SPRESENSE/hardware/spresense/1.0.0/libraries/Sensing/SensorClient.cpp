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


static bool recv00(sensor_command_data_mh_t &dat);
static bool recv01(sensor_command_data_mh_t &dat);
static bool recv02(sensor_command_data_mh_t &dat);
static bool recv03(sensor_command_data_mh_t &dat);
static bool recv04(sensor_command_data_mh_t &dat);
static bool recv05(sensor_command_data_mh_t &dat);
static bool recv06(sensor_command_data_mh_t &dat);
static bool recv07(sensor_command_data_mh_t &dat);
static bool recv08(sensor_command_data_mh_t &dat);
static bool recv09(sensor_command_data_mh_t &dat);
static bool recv10(sensor_command_data_mh_t &dat);
static bool recv11(sensor_command_data_mh_t &dat);
static bool recv12(sensor_command_data_mh_t &dat);
static bool recv13(sensor_command_data_mh_t &dat);
static bool recv14(sensor_command_data_mh_t &dat);
static bool recv15(sensor_command_data_mh_t &dat);
static bool recv16(sensor_command_data_mh_t &dat);
static bool recv17(sensor_command_data_mh_t &dat);
static bool recv18(sensor_command_data_mh_t &dat);
static bool recv19(sensor_command_data_mh_t &dat);
static bool recv20(sensor_command_data_mh_t &dat);
static bool recv21(sensor_command_data_mh_t &dat);
static bool recv22(sensor_command_data_mh_t &dat);
static bool recv23(sensor_command_data_mh_t &dat);
static bool recv24(sensor_command_data_mh_t &dat);


struct funcs_t {
  SensorClient*             p;
  sensor_data_mh_callback_t f;
}
funcs[] = {
  { NULL, recv00 },
  { NULL, recv01 },
  { NULL, recv02 },
  { NULL, recv03 },
  { NULL, recv04 },
  { NULL, recv05 },
  { NULL, recv06 },
  { NULL, recv07 },
  { NULL, recv08 },
  { NULL, recv09 },
  { NULL, recv10 },
  { NULL, recv11 },
  { NULL, recv12 },
  { NULL, recv13 },
  { NULL, recv14 },
  { NULL, recv15 },
  { NULL, recv16 },
  { NULL, recv17 },
  { NULL, recv18 },
  { NULL, recv19 },
  { NULL, recv20 },
  { NULL, recv21 },
  { NULL, recv22 },
  { NULL, recv23 },
  { NULL, recv24 },
};


static bool recv00(sensor_command_data_mh_t &dat)
{
  funcs[ 0].p->subscribe(dat);
}
static bool recv01(sensor_command_data_mh_t &dat)
{
  funcs[ 1].p->subscribe(dat);
}
static bool recv02(sensor_command_data_mh_t &dat)
{
  funcs[ 2].p->subscribe(dat);
}
static bool recv03(sensor_command_data_mh_t &dat)
{
  funcs[ 3].p->subscribe(dat);
}
static bool recv04(sensor_command_data_mh_t &dat)
{
  funcs[ 4].p->subscribe(dat);
}
static bool recv05(sensor_command_data_mh_t &dat)
{
  funcs[ 5].p->subscribe(dat);
}
static bool recv06(sensor_command_data_mh_t &dat)
{
  funcs[ 6].p->subscribe(dat);
}
static bool recv07(sensor_command_data_mh_t &dat)
{
  funcs[ 7].p->subscribe(dat);
}
static bool recv08(sensor_command_data_mh_t &dat)
{
  funcs[ 8].p->subscribe(dat);
}
static bool recv09(sensor_command_data_mh_t &dat)
{
  funcs[ 9].p->subscribe(dat);
}
static bool recv10(sensor_command_data_mh_t &dat)
{
  funcs[10].p->subscribe(dat);
}
static bool recv11(sensor_command_data_mh_t &dat)
{
  funcs[11].p->subscribe(dat);
}
static bool recv12(sensor_command_data_mh_t &dat)
{
  funcs[12].p->subscribe(dat);
}
static bool recv13(sensor_command_data_mh_t &dat)
{
  funcs[13].p->subscribe(dat);
}
static bool recv14(sensor_command_data_mh_t &dat)
{
  funcs[14].p->subscribe(dat);
}
static bool recv15(sensor_command_data_mh_t &dat)
{
  funcs[15].p->subscribe(dat);
}
static bool recv16(sensor_command_data_mh_t &dat)
{
  funcs[16].p->subscribe(dat);
}
static bool recv17(sensor_command_data_mh_t &dat)
{
  funcs[17].p->subscribe(dat);
}
static bool recv18(sensor_command_data_mh_t &dat)
{
  funcs[18].p->subscribe(dat);
}
static bool recv19(sensor_command_data_mh_t &dat)
{
  funcs[19].p->subscribe(dat);
}
static bool recv20(sensor_command_data_mh_t &dat)
{
  funcs[20].p->subscribe(dat);
}
static bool recv21(sensor_command_data_mh_t &dat)
{
  funcs[21].p->subscribe(dat);
}
static bool recv22(sensor_command_data_mh_t &dat)
{
  funcs[22].p->subscribe(dat);
}
static bool recv23(sensor_command_data_mh_t &dat)
{
  funcs[23].p->subscribe(dat);
}
static bool recv24(sensor_command_data_mh_t &dat)
{
  funcs[24].p->subscribe(dat);
}

SensorClient::SensorClient(int      id,
                           uint32_t subscriptions,
                           int      rate,
                           int      sample_watermark_num,
                           int      size_per_sample)
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
      reg.callback_mh     = funcs[id].f;
      SS_SendSensorResister(&reg);

      funcs[id].p = this;
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
