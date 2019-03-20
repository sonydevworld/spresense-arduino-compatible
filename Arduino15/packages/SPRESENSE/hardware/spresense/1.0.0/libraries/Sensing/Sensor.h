
/*
 *  Sensor.h - Sensor include file for the Spresense SDK
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
 * @file Sensor.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Sensor Library Class for Arduino on Spresense.
 * @details By using this library, you can use the follow features on SPRESSENSE.
 *          - Sensing Steps
 */

#ifndef Sensor_h
#define Sensor_h

/**
 * @defgroup Sensor Sensor Library API
 * @brief API for using Sensor
 * @{
 */

#include <pins_arduino.h>

#include <sensing/sensor_api.h>
#include <sensing/sensor_id.h>
#include <sensing/sensor_ecode.h>
#include <sensing/logical_sensor/step_counter.h>

#include <asmp/mpshm.h>

/*--------------------------------------------------------------------------*/
#define print_err printf

#ifdef BRD_DEBUG
#define print_dbg(...) printf(__VA_ARGS__)
#else
#define print_dbg(x...)
#endif

/*--------------------------------------------------------------------------*/
/**
 * Sensor Library Error Code Definitions.
 */
#define SENSORLIB_ECODE_OK                        0x0  /**< Execution result OK. */
#define SENSORLIB_ECODE_ACTIVATE_MANAGER_ERROR    0x2  /**< Failure to activate sensor. */
#define SENSORLIB_ECODE_DEACTIVATE_MANAGER_ERROR  0x3  /**< Failure to dectivate sensor. */
#define SENSORLIB_ECODE_SENSOR_CLIENT_ID_ERROR    0x4  /**< Invalid value for clientID. */
#define SENSORLIB_ECODE_ARGUMENT_NULL_ERROR       0x5  /**< */

#define SENSORLIB_ECODE_CREATE_STEPCOUNTER_ERROR  0x10  /**< Failure to create step counter. */
#define SENSORLIB_ECODE_OPEN_STEPCOUNTER_ERROR    0x11  /**< Failure to open step counter. */
#define SENSORLIB_ECODE_CLOSE_STEPCOUNTER_ERROR   0x12  /**< Failure to close step counter. */

/*--------------------------------------------------------------------------*/
/**
 * Sensor Library Type Definitions.
 */

typedef unsigned int err_t;

/*--------------------------------------------------------------------------*/
/**
 * StepCounter Definitions.
 */

#define STEP_COUNTER_WALKING_STRIDE 60
#define STEP_COUNTER_RUNNING_STRIDE 80

typedef int (*step_counter_notify_result)(FAR StepCounterStepInfo *result);
typedef struct
{
  step_counter_notify_result callback;
  uint8_t walking_stride = STEP_COUNTER_WALKING_STRIDE;
  uint8_t running_stride = STEP_COUNTER_RUNNING_STRIDE;
} step_counter_t;


struct accel_float_s
  {
    float x;  /* X axis standard gravity acceleration.[G] */
    float y;  /* Y axis standard gravity acceleration.[G] */
    float z;  /* Z axis standard gravity acceleration.[G] */
  };
typedef struct accel_float_s accel_float_t;

/*--------------------------------------------------------------------------*/

/**
 * @class SensorClass
 * @brief Sensor Library Class Definitions.
 */
class SensorClass
{
public:

  /**
   * @brief Get instance of SensorClass for singleton.
   */
  static SensorClass* getInstance()
    {
      static SensorClass instance;
      return &instance;
    }

  /**
   * @brief Initialize the Sensor library.
   *
   * @details This function is called only once when using the sensor library.
   *          In this function, initialization of required shared memory management library,
   *          initialization of inter-task communication library, activate processing of
   *          Sensor MW.
   *
   *          If you call it more than once, an error occurs,
   *          but if you call "end ()" you need to call this function again.
   */
  err_t begin(void);

  /**
   * @brief Finalization the Sensor library.
   *
   * @details This function is called when you want to exit the sensor library.
   *          In this function, necessary termination processing of the shared memory management
   *          library, end processing of the inter-task communication library,
   *          deactivate processing of Sensor MW
   *
   *          This can only be called once when activated.
   *          If you call it before calling "begin ()" or call it more than once, an error occurs.
   */
  err_t end(void);

  /**
   * @brief Start sensing of StepCounter
   *
   * @details This function starts sensing of StepCounter.
   *          Once you call this function, the sensor feature will be in the active state,
   *          so you can not call it until you call stopStepCounter.
   *
   *          When StepCounter is started, please starts writing the data of
   *          the accelerometer sensor for sensing.
   */
  err_t startStepCounter(step_counter_t &param);
  
  /**
   * @brief Stop sensing of StepCounter
   *
   * @details This function stops sensing of StepCounter.
   *          The function can be called only when called startStepCounter.
   *
   *          When StepCounter is stoped, please stops writing the data of
   *          the accelerometer sensor for sensing.
   */
  err_t stopStepCounter(void);
  
  /**
   * @brief Write sensing data from buffer
   *
   * @details This function writes from the buffer
   *          to the logical sensor in the sensor library.
   *          It can be called on seinsing is active.
   */
  err_t write_data(
      SensorClientID id,       /** Sensor type. */
      FAR accel_float_t *data, /** Sensor data address. */
      uint32_t sample_freq,    /** Frequency of the sensor data. */
      uint32_t sample_num,     /** Sample number of the sensor data. */
      uint32_t Timestamp       /** Timestamp of the sensor data. */
  );

private:

  SensorClass()
  {}
  SensorClass(const SensorClass&);
  SensorClass& operator=(const SensorClass&);
  ~SensorClass() {}

  template <typename T>
  err_t write_data(
      SensorClientID id,       /** Sensor type. */
      FAR T *data,             /** Sensor data address. */
      uint32_t sample_freq,    /** Frequency of the sensor data. */
      uint32_t sample_num,     /** Sample number of the sensor data. */
      uint32_t Timestamp       /** Timestamp of the sensor data. */
  );

};

extern SensorClass Sensor;

/** @} Sensor */

#endif /* Sensor_h */
