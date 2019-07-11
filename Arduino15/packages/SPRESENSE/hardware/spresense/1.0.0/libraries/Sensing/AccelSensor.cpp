/*
 *  AccelSensor.cpp - Sensor library for the Spresense SDK
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
#include <AccelSensor.h>
#include "Arduino.h"

#define ACCEL_INTERVAL_THRESHOLD  1000  /* [1000ms] */
#define ACCEL_MAX_SAMPLE_NUM        50  /* [50 sample] */


bool AccelSensorClass::begin(int      id,
                             uint32_t subscriptions,
                             int      rate,
                             int      sample_watermark_num,
                             int      size_per_sample)
{
  /* Range check. */

  if (sample_watermark_num > ACCEL_MAX_SAMPLE_NUM)
    {
      printf("Incorrect number of sample_watermark_num.\n");
      return false;
    }

  if (size_per_sample > static_cast<int>(sizeof(struct accel_float_s)))
    {
      printf("Incorrect size of size_per_sample.\n");
      return false;
    }

  /* Init private parameters. */

  if (!SensorClient::begin(id,subscriptions,rate,sample_watermark_num,size_per_sample, NULL))
    {
      return false;
    }

  m_cnt           = 0;
  m_previous_time = millis();
  
  if (ERR_OK != m_mh.allocSeg(
                         S1_SENSOR_DATA_BUF_POOL,
                         size_per_sample * sample_watermark_num))
    {
      /* Fatal error occured. */

      printf("Fail to allocate segment of memory handle.\n");
      return false;
    }
  return true;
}

bool AccelSensorClass::begin(int id,
                             int rate,
                             int sample_watermark_num,
                             int size_per_sample)
{
  return begin(id, 0, rate, sample_watermark_num, size_per_sample);
}

bool AccelSensorClass::end(void)
{
  m_mh.freeSeg();

  return SensorClient::end();
}

int AccelSensorClass::write_data(float x, float y, float z)
{
  /* Check reading cycle */

  long  now           = millis();
  int   read_duration = 1000 / m_rate;  /* 20ms */
  int   diff          = now - m_previous_time;
  
  if (diff <= read_duration)
    {
      /* Do not get data. Because the interval of the cycle is short.
       * Return as a normal end
       */

      return SENSORCLIENT_ECODE_OK;
    }
  
  if (diff >= ACCEL_INTERVAL_THRESHOLD)
    {
       /* Input interval exceeded threshold. Clear the buffer. */

       m_cnt = 0;
       m_previous_time = now;
       printf("Input interval exceeded threshold!\n");
    }
  else
    {
      /* Update previous time */

       m_previous_time += read_duration;
    }

  /* Read raw accelerometer measurements from BMI160 */

  FAR struct accel_float_s *p_src =
    reinterpret_cast<struct accel_float_s *>(m_mh.getPa());

  p_src[m_cnt].x = x;
  p_src[m_cnt].y = y;
  p_src[m_cnt].z = z;
  m_cnt += 1;

  /* Check if the sample for one process has accumulated */

  if (m_cnt == m_sample_watermark_num)
    {
      m_cnt = 0;

      publish(m_mh,
              sizeof(struct accel_float_s),
              m_rate,
              m_sample_watermark_num,
              now);
    
      /* Create new memory buffer. */
      
      if (ERR_OK != m_mh.allocSeg(
                     S1_SENSOR_DATA_BUF_POOL,
                     sizeof(struct accel_float_s) * m_sample_watermark_num))
        {
          /* Fatal error occured. */

          printf("Fail to allocate segment of memory handle.\n");
          assert(0);
        }
    }

  return SENSORCLIENT_ECODE_OK;
}

AccelSensorClass AccelSensor;
