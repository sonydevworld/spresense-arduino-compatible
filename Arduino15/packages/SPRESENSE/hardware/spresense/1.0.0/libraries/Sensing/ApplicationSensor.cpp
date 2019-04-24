/*
 *  ApplicationSensor.cpp - Sensor library for the Spresense SDK
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

#include <ApplicationSensor.h>
#include <Aesm.h>


int ApplicationSensorClass::subscribe(sensor_command_data_mh_t& data)
{
  return reinterpret_cast<int>(data.mh.getVa());
}

int StepCountReaderClass::subscribe(sensor_command_data_mh_t& data)
{
  FAR SensorCmdStepCounter *result_data = 
    reinterpret_cast<SensorCmdStepCounter *>(data.mh.getVa());
  if (SensorOK != result_data->result.exec_result)
    {
      return static_cast<int>NULL;
    }
  if (result_data->exec_cmd.cmd_type != 
            STEP_COUNTER_CMD_UPDATE_ACCELERATION)
    {
      return static_cast<int>NULL;
    }
	
  return reinterpret_cast<int>(&result_data->result.steps);
}

ApplicationSensorClass ApplicationSensor;
StepCountReaderClass   StepCountReader;
