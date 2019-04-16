/*
 *  SensorClient.h - Sensing include file for the Spresense SDK
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
 * @file SensorClient.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Sensor Library Class for Arduino on Spresense.
 * @details By using this library, you can use the follow features
 * on SPRESSENSE.
 *          - Sensing Steps
 */

#ifndef __SENSORCLIENT_H
#define __SENSORCLIENT_H

/**
 * @defgroup Sensor Sensor Library API
 * @brief API for using Sensor
 * @{
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sdk/config.h>
#include <nuttx/init.h>
#include <nuttx/arch.h>
#include <asmp/mpshm.h>

#include <pins_arduino.h>

#include <sensing/sensor_api.h>
#include <sensing/sensor_id.h>
#include <sensing/sensor_ecode.h>

#include <MemoryUtil.h>

#include "memutil/msgq_id.h"
#include "memutil/mem_layout.h"


/**
 * Physical and logical sensor ID definitions.
 */

enum ApplicationFixedSensorClientID
{
  APP_selfID        = selfID,         /*  0 */
  APP_accelID       = accelID,        /*  1 */
  APP_accel1ID      = accel1ID,       /*  2 */
  APP_magID         = magID,          /*  3 */
  APP_pressureID    = pressureID,     /*  4 */
  APP_lightID       = lightID,        /*  5 */
  APP_pulseID       = pulseID,        /*  6 */
  APP_tempID        = tempID,         /*  7 */
  APP_gyroID        = gyroID,         /*  8 */
  APP_gnssID        = gnssID,         /*  9 */
  /* Fixed ID */
  APP_stepcounterID = stepcounterID,  /* 10 */
  APP_tramID        = tramID,         /* 11 */
  APP_gestureID     = gestureID,      /* 12 */
  APP_compassID     = compassID,      /* 13 */
  APP_barometerID   = barometerID,    /* 14 */
  APP_tramliteID    = tramliteID,     /* 15 */
  APP_vadID         = vadID,          /* 16 */
  APP_wuwsrID       = wuwsrID,        /* 17 */
  APP_adcID         = adcID,          /* 18 */
  APP_reserve19ID   = reserve19ID,    /* 19 */
  APP_app0ID        = app0ID,         /* 20 */
  APP_app1ID        = app1ID,         /* 21 */
  APP_app2ID        = app2ID,         /* 22 */
  APP_app3ID        = app3ID,         /* 23 */
  APP_ID_MAX        = NumOfSensorClientID,
};


/**
 * @class SensorClientClass
 * @brief Sensor Library Class Definitions.
 */

class SensorClient
{
public:
  SensorClient(int      id,
               uint32_t subscriptions        = 0,
               int      rate                 = 0,
               int      sample_watermark_num = 0,
               int      size_per_sample      = 0,
               sensor_data_mh_callback_t cb  =NULL);

  /**
   * @brief Write sensing data from buffer
   *
   * @details This function writes from the buffer
   *          to the logical sensor in the sensor library.
   *          It can be called on seinsing is active.
   */

  int publish(
      PoolId    id,                   /** Memory pool ID. */
      FAR void* data,                 /** Sensor data address. */
      uint32_t  size_per_sample,      /** Sample size of the sensor data. */
      uint32_t  freq,                 /** Frequency of the sensor data. */
      uint32_t  sample_watermark_num, /** Sample number of the sensor
                                       *  data. */
      uint32_t  timestamp);           /** Timestamp of the sensor data. */

  /**
   * @brief Read sensing data from buffer
   *
   * @details This function Called when subscribed from the logical sensor.
   */

  virtual int subscribe(sensor_command_data_mh_t& data)
  {
    return 0;
  }

protected:
  int m_id;
  int m_rate;
  int m_sample_watermark_num; 
  int m_size_per_sample;

};

#endif /* __SENSORCLIENT_H */
