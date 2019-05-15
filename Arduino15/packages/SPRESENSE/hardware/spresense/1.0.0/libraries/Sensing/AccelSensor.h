/*
 *  AccelSensor.h - Sensing include file for the Spresense SDK
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

/**
 * @file AccelSensor.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Sensor Library Class for Arduino on Spresense.
 * @details By using this library, you can use the follow features
 * on SPRESENSE.
 *          - Sensing Steps
 */

#ifndef __ACCELSENSOR_H
#define __ACCELSENSOR_H

#include <SensorClient.h>


class AccelSensorClass : public SensorClient
{
public:
  bool begin(int      id,
             uint32_t subscriptions        = 0,
             int      rate                 = 0,
             int      sample_watermark_num = 0,
             int      size_per_sample      = 0);
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
  unsigned long         m_previous_time;
  MemMgrLite::MemHandle m_mh;

};

extern AccelSensorClass AccelSensor;

#endif /* __ACCELSENSOR_H */