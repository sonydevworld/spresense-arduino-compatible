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

#include <ApplicationSensor.h>
#include <AccelSensor.h>
#include <StepCounterSensor.h>
#include <App0Sensor.h>


/* Physical & logical Sensor ID */
enum {
  ACCEL_ID       = 1,
  STEPCOUNTER_ID = stepcounterID,
  APP0_ID,
};


/* Static variable */

static ApplicationSensor* theAccelSensor;
static ApplicationSensor* theStepCounterSensor;
static ApplicationSensor* theApp0Sensor;


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


/****************************************************************************
 * Callback function for Sensor Class
 ****************************************************************************/
static bool step_counter_receive_result(sensor_command_data_mh_t& data)
{
  FAR SensorCmdStepCounter *result_data =
    reinterpret_cast<SensorCmdStepCounter *>(data.mh.getVa());
  if (SensorOK == result_data->result.exec_result)
    {
      if (result_data->exec_cmd.cmd_type == 
            STEP_COUNTER_CMD_UPDATE_ACCELERATION)
        {
          /* Call application callback */

          step_counter_result(&result_data->result.steps);
        }
    }

  return true;
}


/**
 * @brief Initialize StepCounter
 */
void setup()
{
  /* Initialize sensor class */

  theAccelSensor       = new AccelSensor(ACCEL_ID);
  theStepCounterSensor = new StepCounterSensor(
                                         STEPCOUNTER_ID,
                                         1 << ACCEL_ID);
  theApp0Sensor        = new App0Sensor( APP0_ID,
                                         1 << STEPCOUNTER_ID,
                                         step_counter_receive_result);
}

/**
 * @brief Sensing process
 */
void loop()
{
  theAccelSensor->write_data((void*)millis());
}
