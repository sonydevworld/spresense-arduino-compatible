/*
 *  ApplicationSensor.cpp - SPI implement file for the Spresense SDK
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


ApplicationSensor::ApplicationSensor(
              int                       id,
              uint32_t                  subscriptions,
              application_sensor_notify callback) :
  SensorClient(id, subscriptions)
{
  m_callback = callback;
}


int ApplicationSensor::subscribe(sensor_command_data_mh_t& data)
{
  (*m_callback)(data.get_self(), data.mh.getVa());

  return 0;
}
