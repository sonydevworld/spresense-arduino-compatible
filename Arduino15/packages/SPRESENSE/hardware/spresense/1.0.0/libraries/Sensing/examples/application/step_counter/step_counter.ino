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
#include <SensorManager.h>
#include <AccelSensor.h>
#include <Aesm.h>
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
bool step_counter_result(sensor_command_data_mh_t &data)
{
  /* Display result of sensing */

  StepCounterStepInfo* steps =
              reinterpret_cast<StepCounterStepInfo*>
              (StepCountReader.subscribe(data));
  
  if (steps == NULL)
    {
      return 0;
    }

  float  tempo = 0;

  switch (steps->movement_type)
    {
      case STEP_COUNTER_MOVEMENT_TYPE_WALK:
      case STEP_COUNTER_MOVEMENT_TYPE_RUN:
        tempo = steps->tempo;
        break;
      case STEP_COUNTER_MOVEMENT_TYPE_STILL:

        /* In this state, the tempo value on the display is zero. */

        tempo = 0;

        break;
      default:

        /* It is not displayed in the state other than the above. */

        return 0;
    }

  printf("%11.5f,%11.2f,%11.5f,%11.5f,%11ld,",
               tempo,
               steps->stride,
               steps->speed,
               steps->distance,
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

  SensorManager.begin();

  AccelSensor.begin(SEN_accelID,
                    accel_rate,
                    accel_sample_num,
                    accel_sample_size);

  Aesm.begin(SEN_stepcounterID,
             SUBSCRIPTION(SEN_accelID),
             accel_rate,
             accel_sample_num,
             accel_sample_size);

  StepCountReader.begin(SEN_app0ID,
                        SUBSCRIPTION(SEN_stepcounterID),
                        step_counter_result);

  /* Initialize StepCounter parameters */

  Aesm.set(walking_stride, running_stride);


  puts("Start sensing...");
  puts("-----------------------------------------------------------------------");
  puts("      tempo,     stride,      speed,   distance,       step,  move-type");
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

  AccelSensor.write_data(x, y, z);

}
