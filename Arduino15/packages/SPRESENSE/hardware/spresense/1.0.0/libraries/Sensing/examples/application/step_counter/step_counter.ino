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

#include <MemoryUtil.h>
#include <SensorSystem.h>
#include <AccelSensor.h>
#include <StepCounterSensor.h>
#include <ApplicationSensor.h>


/* Const values */

const int i2c_addr                = 0x68;
const int baudrate                = 115200;

const int walking_stride          = 60; /* 60cm */
const int running_stride          = 80; /* 80cm */

const int accel_range             =  2; /* 2G */
const int accel_rate              = 50; /* 50 Hz */
const int accel_sample_num        = 50; /* 50 sample */
const int accel_sample_size       = sizeof(float) * 3;

/**
 * @brief Call result of sensing
 */
unsigned char step_counter_result(sensor_command_data_t &dat)
{
  
  FAR SensorCmdStepCounter *result_data = 
    reinterpret_cast<SensorCmdStepCounter *>(dat.adr);
  if (SensorOK != result_data->result.exec_result)
    {
      return 1;
    }
  if (result_data->exec_cmd.cmd_type != 
            STEP_COUNTER_CMD_UPDATE_ACCELERATION)
    {
      return 1;
    }

  /* Display result of sensing */

  StepCounterStepInfo* steps = &result_data->result.steps;

  printf("   %8ld,   %8ld,   %8ld,   %8ld,   %8lld,   %8ld,",
               (uint32_t)steps->tempo,
               (uint32_t)steps->stride,
               (uint32_t)steps->speed,
               (uint32_t)steps->distance,
               steps->time_stamp,
               steps->step);

  switch (steps->movement_type)
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


/* Static variable */

static AccelSensor*       theAccelSensor;
static StepCounterSensor* theStepCounterSensor;
static ApplicationSensor* theApplicationSensor;

unsigned char step_counter_cb(sensor_command_data_mh_t &dat)
{
  return theStepCounterSensor->subscribe(dat);
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

  MemoryUtil.begin();
  MemoryUtil.setLayout(MEM_LAYOUT_SENSORS);

  SensorSystem.begin();

  theAccelSensor       = new AccelSensor(APP_accelID,
                                 0,
                                 accel_rate,
                                 accel_sample_num,
                                 accel_sample_size);

  theStepCounterSensor = new StepCounterSensor(
                                 APP_stepcounterID,
                                 1 << APP_accelID,
                                 accel_rate,
                                 accel_sample_num,
                                 accel_sample_size,
                                 step_counter_cb);

  theApplicationSensor = new ApplicationSensor(
                                 APP_app0ID,
                                 1 << APP_stepcounterID,
                                 step_counter_result);

  /* Initialize StepCounter parameters */

  theStepCounterSensor->set(walking_stride, running_stride);


  puts("Start sensing...");
  puts("-------------------------------------------------------------------------------------");
  puts("      tempo,     stride,      speed,   distance,    t-stamp,       step,  move-type");
}

/**
 * @brief Sensing process
 */
void loop()
{
  float x;
  float y;
  float z;

  /* Read raw accelerometer measurements from BMI160 */

  BMI160.readAccelerometerScaled(x, y, z);

  theAccelSensor->write_data(x, y, z);

}
