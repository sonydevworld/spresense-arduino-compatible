/*
 *  StepCounterSensor.cpp - SPI implement file for the Spresense SDK
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

#include <StepCounterSensor.h>


/**
 * StepCounter Definitions.
 */

#define STEP_COUNTER_WALKING_STRIDE     60
#define STEP_COUNTER_RUNNING_STRIDE     80


/* Convert table 50Hz to 32Hz */

char freq_convert_table[] =
{
   0,  1,  3,  4,  6,  7,  9, 10, 12, 13, 15, 16, 18, 19, 21, 23,
  25, 26, 28, 29, 31, 32, 34, 35, 37, 38, 40, 41, 43, 44, 46, 48
};


StepCounterSensor::StepCounterSensor(
                       int      id,
                       uint32_t subscriptions,
                       int      rate,
                       int      sample_watermark_num,
                       int      size_per_sample,
                       int      input_rate,
                       int      input_sample_watermark_num,
                       int      input_size_per_sample) : 
  SensorClient(id,
               subscriptions,
               rate,
               sample_watermark_num,
               size_per_sample)
{
  m_input_rate                 = input_rate;
  m_input_sample_watermark_num = input_sample_watermark_num;
  m_input_size_per_sample      = input_size_per_sample;

  step_counter_ins = StepCounterCreate(SENSOR_DSP_CMD_BUF_POOL);
  assert(step_counter_ins);

  StepCounterOpen(step_counter_ins);

  set(STEP_COUNTER_WALKING_STRIDE, STEP_COUNTER_RUNNING_STRIDE);
}


int StepCounterSensor::set(uint8_t walking_stride,
                           uint8_t running_stride)
{
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


int StepCounterSensor::subscribe(sensor_command_data_mh_t& data)
{
  FAR char* pSrc = 
      reinterpret_cast<char*>(data.mh.getVa());

  MemMgrLite::MemHandle mh;

  /* Allocate memory for output. */

  if (ERR_OK != mh.allocSeg(
                   STEP_DATA_BUF_POOL,
                   m_size_per_sample * m_sample_watermark_num))
    {
      /* Fatal error occured. */

      printf("Fail to allocate segment of memory handle.\n");
      return ERR_NG;
    }
  FAR char *pDst = reinterpret_cast<char *>(mh.getPa());

  /* Convert data from input to output. */

  for (int i = 0; i < m_sample_watermark_num; i++)
    {
      memcpy(
        pDst,
        &pSrc[freq_convert_table[i] * m_input_size_per_sample],
        m_size_per_sample);

      pDst += m_size_per_sample;
    }

  /* Free input data. */

  data.mh.freeSeg();

  /* Change output params. */

  data.mh   = mh;
  data.fs   = m_rate;
  data.size = m_sample_watermark_num;

  StepCounterWrite(step_counter_ins, &data);

  return 0;
}
