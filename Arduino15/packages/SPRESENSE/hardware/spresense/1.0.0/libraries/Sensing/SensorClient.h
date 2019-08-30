/*
 *  SensorClient.h - Sensing include file for the Spresense SDK
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
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

/**
 * @file SensorClient.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Sensor Library Class for Arduino on Spresense.
 * @details By using this library, you can use the follow features
 * on SPRESENSE.
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


/**
 * Sensor Library Error Code Definitions.
 */

/**< Executionresult OK. */

#define SENSORCLIENT_ECODE_OK                       0x0

/**< Failure to activate sensor. */

#define SENSORCLIENT_ECODE_ACTIVATE_MANAGER_ERROR   0x1

/**< Failure to dectivate sensor. */

#define SENSORCLIENT_ECODE_DEACTIVATE_MANAGER_ERROR 0x2

/**< Invalid value for clientID. */

#define SENSORCLIENT_ECODE_SENSOR_CLIENT_ID_ERROR   0x3  

/**< Memory allocation failure */

#define SENSORCLIENT_ECODE_MEMORY_ALLOCATE_ERROR    0x4  


/**
 * Physical and logical sensor ID definitions.
 */

enum GeneralSensorClientID
{
  SensorClientID00,   /*  0 */
  SensorClientID01,   /*  1 */
  SensorClientID02,   /*  2 */
  SensorClientID03,   /*  3 */
  SensorClientID04,   /*  4 */
  SensorClientID05,   /*  5 */
  SensorClientID06,   /*  6 */
  SensorClientID07,   /*  7 */
  SensorClientID08,   /*  8 */
  SensorClientID09,   /*  9 */
  SensorClientID10,   /* 10 */
  SensorClientID11,   /* 11 */
  SensorClientID12,   /* 12 */
  SensorClientID13,   /* 13 */
  SensorClientID14,   /* 14 */
  SensorClientID15,   /* 15 */
  SensorClientID16,   /* 16 */
  SensorClientID17,   /* 17 */
  SensorClientID18,   /* 18 */
  SensorClientID19,   /* 19 */
  SensorClientID20,   /* 20 */
  SensorClientID21,   /* 21 */
  SensorClientID22,   /* 22 */
  SensorClientID23,   /* 23 */
  NumOfGeneralSensorClientID,
};

/**
 * Physical and logical sensor ID definitions.
 */

enum
{
  SEN_selfID        = SensorClientID00, /*  0 */
  SEN_accelID       = SensorClientID01, /*  1 */
  SEN_accel1ID      = SensorClientID02, /*  2 */
  SEN_magID         = SensorClientID03, /*  3 */
  SEN_pressureID    = SensorClientID04, /*  4 */
  SEN_lightID       = SensorClientID05, /*  5 */
  SEN_pulseID       = SensorClientID06, /*  6 */
  SEN_tempID        = SensorClientID07, /*  7 */
  SEN_gyroID        = SensorClientID08, /*  8 */
  SEN_gnssID        = SensorClientID09, /*  9 */
  SEN_stepcounterID = SensorClientID10, /* 10 */
  SEN_tramID        = SensorClientID11, /* 11 */
  SEN_gestureID     = SensorClientID12, /* 12 */
  SEN_compassID     = SensorClientID13, /* 13 */
  SEN_barometerID   = SensorClientID14, /* 14 */
  SEN_tramliteID    = SensorClientID15, /* 15 */
  SEN_vadID         = SensorClientID16, /* 16 */
  SEN_wuwsrID       = SensorClientID17, /* 17 */
  SEN_adcID         = SensorClientID18, /* 18 */
  SEN_reserve19ID   = SensorClientID19, /* 19 */
  SEN_app0ID        = SensorClientID20, /* 20 */
  SEN_app1ID        = SensorClientID21, /* 21 */
  SEN_app2ID        = SensorClientID22, /* 22 */
  SEN_app3ID        = SensorClientID23, /* 23 */
  SEN_ID_MAX        = NumOfGeneralSensorClientID,
};


/* Convert SenserClientID to SubscriptionID */

#define SUBSCRIPTION(x) (1 << (x))


/**
 * @class SensorClientClass
 * @brief Sensor Library Class Definitions.
 */

class SensorClient
{
public:

  /**
   * @brief Register sensor ID.
   *
   */
  bool begin(int      id,
            uint32_t subscriptions        = 0,
            int      rate                 = 0,
            int      sample_watermark_num = 0,
            int      size_per_sample      = 0,
            sensor_data_mh_callback_t cb  =NULL);

  bool begin(int      id,
             uint32_t subscriptions,
             sensor_data_mh_callback_t cb);

  /**
   * @brief Release sensor ID.
   *
   */
  bool end(void);

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
   * @brief Write sensing data from MemHandle
   *
   */

  int publish(
      MemMgrLite::MemHandle& mh,
      uint32_t               size_per_sample,
      uint32_t               freq,
      uint32_t               sample_watermark_num,
      uint32_t               timestamp);

  int publish(FAR void      *data,
              uint32_t       size_per_sample,
              uint32_t       freq,
              uint32_t       sample_watermark_num,
              uint32_t       timestamp);

  /**
   * @brief Read sensing data from buffer
   *
   * @details This function Called when subscribed from the logical sensor.
   */

  void *subscribe(sensor_command_data_mh_t& data)
  {
    return data.mh.getVa();
  }

protected:
  int m_id;
  int m_rate;
  int m_sample_watermark_num; 
  int m_size_per_sample;

};

/** @} Sensor */

#endif /* __SENSORCLIENT_H */
