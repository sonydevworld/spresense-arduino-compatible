/*
 *  SensorManager.h - Sensing include file for the Spresense SDK
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
 * @file SensorManager
 * @author Sony Semiconductor Solutions Corporation
 * @brief Sensor Library Class for Arduino on Spresense.
 * @details By using this library, you can use the follow features
 * on SPRESENSE.
 *          - Sensing Steps
 */

#include <sensing/sensor_api.h>
#include <sensing/sensor_id.h>
#include <sensing/sensor_ecode.h>

#include <SensorManager.h>
#include <MemoryUtil.h>

SensorManagerClass SensorManager;

/****************************************************************************
 * Callback function for Sensor Class
 ****************************************************************************/
static void sensor_manager_api_response(unsigned int code,
                                        unsigned int ercd,
                                        unsigned int self)
{
  if (ercd != SS_ECODE_OK)
    {
      printf("Error: get api response. code %d, ercd %d, self %d\n",
                code, ercd, self);
    }

  return;
}

/****************************************************************************
 * 
 ****************************************************************************/

bool SensorManagerClass::begin()
{
  MemoryUtil.begin();
  MemoryUtil.setLayout(MEM_SECTION_SENSOR, MEM_LAYOUT_SENSORS);

  return SS_ActivateSensorSubSystem(MSGQ_SEN_MGR, sensor_manager_api_response);
}

bool SensorManagerClass::end()
{
  return SS_DeactivateSensorSubSystem();
}
