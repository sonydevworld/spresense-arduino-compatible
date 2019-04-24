/*
 *  ApplicationSensor.h - Sensing include file for the Spresense SDK
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
 * @file ApplicationSensor.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Sensor Library Class for Arduino on Spresense.
 * @details By using this library, you can use the follow features
 * on SPRESENSE.
 *          - Sensing Steps
 */

#ifndef __APPLICATIONSENSOR_H
#define __APPLICATIONSENSOR_H

#include <SensorClient.h>


class ApplicationSensorClass : public SensorClient
{
public:

  int subscribe(sensor_command_data_mh_t& data);

private:

};

class StepCountReaderClass : public ApplicationSensorClass
{
public:

  int subscribe(sensor_command_data_mh_t& data);

private:

};

extern ApplicationSensorClass ApplicationSensor;
extern StepCountReaderClass   StepCountReader;


#endif /* __APPLICATIONSENSOR_H */
