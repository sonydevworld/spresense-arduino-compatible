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
#include <ApplicationSensor.h>
#include <sensing/logical_sensor/step_counter.h>


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


/* Convert table 50Hz to 32Hz */

char freq_convert_table[step_counter_rate] =
{
  0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 18, 19, 21, 23, 25,
  26, 28, 29, 31, 32, 34, 35, 37, 38, 40, 41, 43, 44, 46, 48
};


/* AccelSensor class */

class AccelSensor : public ApplicationSensor
{
public:
  AccelSensor() : ApplicationSensor(APP_accelID, 0) {}
};


/* StepCounterSensor class */

class StepCounterSensor : public ApplicationSensor
{
public:
  StepCounterSensor() :
    ApplicationSensor(APP_stepcounterID, 1 << APP_accelID)
  {
    step_counter_ins = StepCounterCreate(SENSOR_DSP_CMD_BUF_POOL);
    assert(step_counter_ins);

    StepCounterOpen(step_counter_ins);

    /* Setup Stride setting.
     * The range of configurable stride lenght is 1 - 249[cm].
     * For the mode, set STEP_COUNTER_MODE_FIXED_LENGTH fixed.
     */

    StepCounterSetting set;
    set.walking.step_length = walking_stride;
    set.walking.step_mode   = STEP_COUNTER_MODE_FIXED_LENGTH;
    set.running.step_length = running_stride;
    set.running.step_mode   = STEP_COUNTER_MODE_FIXED_LENGTH;
    StepCounterSet(step_counter_ins, &set);
  }

  int subscribe(sensor_command_data_mh_t& data)
  {
    StepCounterWrite(step_counter_ins, &data);
  }

private:
  FAR StepCounterClass *step_counter_ins;

};


/* Output class */

class Output : public ApplicationSensor
{
public:
  Output() : ApplicationSensor(APP_app0ID, 1 << APP_stepcounterID)
  {
    puts("Start sensing...");
    puts("-------------------------------------------------------------------------------------");
    puts("      tempo,     stride,      speed,   distance,    t-stamp,       step,  move-type");
  }

  int subscribe(sensor_command_data_mh_t& data)
  {
    FAR SensorCmdStepCounter *result_data =
      reinterpret_cast<SensorCmdStepCounter *>(data.mh.getVa());
    if (SensorOK == result_data->result.exec_result)
      {
        if (result_data->exec_cmd.cmd_type == 
              STEP_COUNTER_CMD_UPDATE_ACCELERATION)
          {
            /*  */

            printf("   %8ld,   %8ld,   %8ld,   %8ld,   %8lld,   %8ld,",
                   (uint32_t)result_data->result.steps.tempo,
                   (uint32_t)result_data->result.steps.stride,
                   (uint32_t)result_data->result.steps.speed,
                   (uint32_t)result_data->result.steps.distance,
                   result_data->result.steps.time_stamp,
                   result_data->result.steps.step);

            switch (result_data->result.steps.movement_type)
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
          }
      }
    return 0;
  }
};


/* Static variable */

static ApplicationSensor* theAccelSensor;



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

  theAccelSensor       = new AccelSensor;
                         new StepCounterSensor;
                         new Output;

}

/**
 * @brief Sensing process
 */
void loop()
{
  /* Static variable for sensing process */

  struct accel_float_s
    {
      float x;  /* X axis standard gravity acceleration.[G] */
      float y;  /* Y axis standard gravity acceleration.[G] */
      float z;  /* Z axis standard gravity acceleration.[G] */
    };
  static int cnt = 0;
  static struct accel_float_s data[step_counter_rate];
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
      struct accel_float_s *p_data = data;

      /* Convert frequency */

      for(int i = 0; i < step_counter_sample; i++, p_data++)
        {
          p_data->x = x[freq_convert_table[i]];
          p_data->y = y[freq_convert_table[i]];
          p_data->z = z[freq_convert_table[i]];
        }

      /* Write Accelarometer data to SensorClass */

      theAccelSensor->publish(data,
                              sizeof(struct accel_float_s),
                              step_counter_rate,
                              step_counter_sample,
                              now);
    }

  /* Update previous time */

  previous_time = now;
}
