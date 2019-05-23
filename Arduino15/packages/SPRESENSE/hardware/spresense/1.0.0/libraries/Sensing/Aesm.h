/*
 *  Aesm.h - Sensing include file for the Spresense SDK
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
 * @file Aesm.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Sensor Library Class for Arduino on Spresense.
 * @details By using this library, you can use the follow features
 * on SPRESENSE.
 *          - Sensing Steps
 */

#ifndef __AESM_H
#define __AESM_H

#include <SensorClient.h>
#include <sensing/logical_sensor/step_counter.h>


/**
 * Aesm Class Error Code Definitions.
 */

/**< Failure to create step counter. */

#define AESM_ECODE_CREATE_ERROR  0x10

/**< Failure to open step counter. */

#define AESM_ECODE_OPEN_ERROR    0x11

/**< Failure to close step counter. */

#define AESM_ECODE_CLOSE_ERROR   0x12

/**< Failure to set step counter. */

#define AESM_ECODE_SET_ERROR     0x13


class AesmClass : public SensorClient
{
public:

  /**
   * @brief Start sensing of StepCounter
   *
   */

  bool begin(int      id,
             uint32_t subscriptions,
             int      input_rate,
             int      input_sample_watermark_num,
             int      input_size_per_sample);

  /**
   * @brief Stop sensing of StepCounter
   *
   */

  bool end();

  int subscribe(sensor_command_data_mh_t& data);

  int set(uint8_t walking_stride, uint8_t running_stride);

private:
  int startAesm(void);

private:
  int m_input_rate;
  int m_input_sample_watermark_num;
  int m_input_size_per_sample;


  FAR StepCounterClass *step_counter_ins;

};

extern AesmClass Aesm;

#endif /* __AESM_H */
