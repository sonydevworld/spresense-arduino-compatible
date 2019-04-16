/*
 *  AccelSensor.cpp - SPI implement file for the Spresense SDK
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
#include <AccelSensor.h>
#include "Arduino.h"


AccelSensor::AccelSensor(int      id,
                         uint32_t subscriptions,
                         int      rate,
                         int      sample_watermark_num,
                         int      size_per_sample) :
  SensorClient(id,
               subscriptions,
               rate,
               sample_watermark_num,
               size_per_sample,
               NULL)
{
  /* Init private parameters. */

  m_cnt           = 0;
  m_previous_time = millis();
  m_adjust_time   = 0;
}


int AccelSensor::write_data(float x, float y, float z)
{
  /* Check reading cycle */

  long  now           = millis();
  int   read_duration = 1000 / m_rate;

  if ((now - m_previous_time) <= (read_duration - m_adjust_time))
    {
      return ERR_NG;
    }

  m_adjust_time = now - m_previous_time - read_duration;
  if (m_adjust_time > read_duration)
    {
      m_adjust_time = 0;
    }

  /* Read raw accelerometer measurements from BMI160 */

  assert(m_sample_watermark_num < MAX_SAMPLE_NUM);

  m_data[m_cnt].x = x;
  m_data[m_cnt].y = y;
  m_data[m_cnt].z = z;
  m_cnt += 1;

  /* Check if the sample for one process has accumulated */

  if (m_cnt == m_sample_watermark_num)
    {
      m_cnt = 0;

      publish(ACCEL_DATA_BUF_POOL,
              m_data,
              sizeof(struct accel_float_s),
              m_rate,
              m_sample_watermark_num,
              now);
    }

  /* Update previous time */

  m_previous_time = now;

  return ERR_OK;
}
