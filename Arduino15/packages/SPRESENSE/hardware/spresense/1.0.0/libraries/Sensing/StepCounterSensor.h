/*
 *  StepCounterSensor.h - Sensing include file for the Spresense SDK
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

/**
 * @file StepCounterSensor.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Sensor Library Class for Arduino on Spresense.
 * @details By using this library, you can use the follow features
 * on SPRESSENSE.
 *          - Sensing Steps
 */

#ifndef __STEPCOUNTERSENSOR_H
#define __STEPCOUNTERSENSOR_H

#include <SensorClient.h>
#include <sensing/logical_sensor/step_counter.h>


class StepCounterSensor : public SensorClient
{
public:
  StepCounterSensor(
               int      id,
               uint32_t subscriptions,
               int      input_rate,
               int      input_sample_watermark_num,
               int      input_size_per_sample,
               sensor_data_mh_callback_t cb);

  int subscribe(sensor_command_data_mh_t& data);

  int set(uint8_t walking_stride, uint8_t running_stride);


private:
  int m_input_rate;
  int m_input_sample_watermark_num;
  int m_input_size_per_sample;


	FAR StepCounterClass *step_counter_ins;

};
#endif /* __STEPCOUNTERSENSOR_H */
