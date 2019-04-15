/*
 *  AccelSensor.h - Sensing include file for the Spresense SDK
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

/**
 * @file AccelSensor.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Sensor Library Class for Arduino on Spresense.
 * @details By using this library, you can use the follow features
 * on SPRESSENSE.
 *          - Sensing Steps
 */

#ifndef __ACCELSENSOR_H
#define __ACCELSENSOR_H

#include <SensorClient.h>


#define MAX_SAMPLE_NUM    (128)


class AccelSensor : public SensorClient
{
public:
  AccelSensor(int      id,
              uint32_t subscriptions,
              int      rate,
              int      sample_watermark_num,
              int      size_per_sample);

  /**
   * @brief 1 Sample data write.
   */

  int write_data(float x, float y, float z);

private:
  struct accel_float_s
    {
      float x;  /* X axis standard gravity acceleration.[G] */
      float y;  /* Y axis standard gravity acceleration.[G] */
      float z;  /* Z axis standard gravity acceleration.[G] */
    };

  int                   m_cnt;                   /* private counter */
  struct accel_float_s  m_data[MAX_SAMPLE_NUM];
  unsigned long         m_previous_time;
  unsigned long         m_adjust_time;

};

#endif /* __ACCELSENSOR_H */
