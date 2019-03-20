/*
 *  step_counter.ino - Step Conter example application
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

#include <BMI160Gen.h>
#include <Sensor.h>
#include <MemoryUtil.h>

/* Const values */

const int i2c_addr = 0x68;
const int baudrate = 115200;

const int walking_stride = 60; /* 60cm */
const int running_stride = 80; /* 80cm */

const int accel_range  = 2;  /* 2G */
const int accel_rate   = 50; /* 50 Hz */
const int accel_sample = 50; /* 50 sample */

const int step_counter_rate   = 32; /* 32 Hz */
const int step_counter_sample = 32; /* 32sample/1process */

const int read_duration = 1000 / accel_rate;

/* Static variable */

static SensorClass *theSensor;

/* Convert table 50Hz to 32Hz */

char freq_convert_table[step_counter_rate] =
{
  0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 18, 19, 21, 23, 25,
  26, 28, 29, 31, 32, 34, 35, 37, 38, 40, 41, 43, 44, 46, 48
};

/**
 * @brief Display result of sensing
 */
int step_counter_result(StepCounterStepInfo *result)
{
  printf("   %8ld,   %8ld,   %8ld,   %8ld,   %8lld,   %8ld,",
         (uint32_t)result->tempo,
         (uint32_t)result->stride,
         (uint32_t)result->speed,
         (uint32_t)result->distance,
         result->time_stamp,
         result->step);

  switch (result->movement_type)
    {
      case STEP_COUNTER_MOVEMENT_TYPE_STILL:
        puts("   stopping");
        break;
      case STEP_COUNTER_MOVEMENT_TYPE_WALK:
        puts("   walking");
        break;
      case STEP_COUNTER_MOVEMENT_TYPE_RUN:
        puts("   running");
        break;
      default:
        puts("   UNKNOWN");
        break;
    }
 return 0;
}

/**
 * @brief Initialize StepCounter
 */
void setup()
{
  /* Initialize Serial communication. */

  Serial.begin(baudrate);

  /* Wait for the serial port to open. */

  while (!Serial);

  /* Initialize device. */

  BMI160.begin(BMI160GenClass::I2C_MODE, i2c_addr);

  /* Set device setting */

  BMI160.setAccelerometerRange(accel_range);
  BMI160.setAccelerometerRate(accel_rate);

  /* Initialize sensor class */

  theSensor = SensorClass::getInstance();
  theSensor->begin();

  /* Set stride of StepCounter. */

  step_counter_t param;
  param.callback = step_counter_result;
  param.walking_stride = walking_stride;
  param.running_stride = running_stride;
  theSensor->startStepCounter(param);

  puts("Start sensing...");
  puts("-------------------------------------------------------------------------------------");
  puts("      tempo,     stride,      speed,   distance,    t-stamp,       step,  move-type");
}
static int process_flag = 1;
/**
 * @brief Sensing process
 */
void loop()
{
if(process_flag != 1)
return;

  /* Static variable for sensing process */

  static int cnt = 0;
  static accel_float_t data[step_counter_rate];
  static float x[accel_sample];
  static float y[accel_sample];
  static float z[accel_sample];
  static unsigned long previous_time = millis();
  static unsigned long adjust_time = 0;

  /* Check reading cycle */

  unsigned long now = millis();
  if ((now - previous_time) <= (read_duration - adjust_time))
    {
      return;
    }

  adjust_time = now - previous_time - read_duration;
  if (adjust_time > read_duration)
    {
      adjust_time = 0;
    }

  /* Read raw accelerometer measurements from BMI160 */

  BMI160.readAccelerometerScaled(x[cnt], y[cnt], z[cnt]);
  cnt += 1;

  /* Check if the sample for one process has accumulated */

  if (cnt == accel_sample)
    {
      cnt = 0;
      accel_float_t *p_data = data;

      /* Convert frequency */

      for(int i = 0; i < step_counter_sample; i++, p_data++)
        {
          p_data->x = x[freq_convert_table[i]];
          p_data->y = y[freq_convert_table[i]];
          p_data->z = z[freq_convert_table[i]];
        }

      /* Write Accelarometer data to SensorClass */

      theSensor->write_data(accelID,
                            data,
                            step_counter_rate,
                            step_counter_sample,
                            now);
    }

  /* Update previous time */

  previous_time = now;
}
