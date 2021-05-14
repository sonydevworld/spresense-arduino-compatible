/*
 *  IIR.cpp - IIR(biquad cascade) Library
 *  Copyright 2019, 2021 Sony Semiconductor Solutions Corporation
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

#include "IIR.h"

#include <stdio.h>

bool IIRClass::begin(filterType_t type, int channel, int cutoff, float q, int sample, format_t output, int fs)
{
  if ((cutoff <= 0) || (cutoff >= fs)){
    m_err = ERR_FS;
    return false;
  }

  if (channel > MAX_CHANNEL_NUM) {
    m_err = ERR_CH_NUM;
    return false;
  }

  if (sample < MIN_FRAMESIZE) {
      m_err = ERR_FRAME_SIZE;
      return false;
  }

  m_channel = channel;
  m_framesize = sample;
  m_output = output;
  m_fs = fs;

  if (create_coef(type, cutoff, q) == false) {
    m_err = ERR_FILTER_TYPE;
    return false;
  }

  for (int i = 0; i < m_channel; i++) {
    m_ringbuff[i] = new RingBuff(channel * sizeof(q15_t) * sample * INPUT_BUFFER_SIZE);
    if (!m_ringbuff[i]) {
      m_err = ERR_MEMORY;
      goto error_return;
    }
  }

  /* Temporary buffer */
  m_tmpInBuff  = new float[m_framesize];
  m_tmpOutBuff = new float[m_framesize];
  if ((!m_tmpInBuff) || (!m_tmpInBuff)) {
    m_err = ERR_MEMORY;
    goto error_return;
  }

  if (m_output == Interleave) {
    m_InterleaveBuff = new q15_t[m_framesize];
    if (!m_InterleaveBuff) {
      m_err = ERR_MEMORY;
      goto error_return;
    }
  }

  for (int i = 0; i < channel; i++) {
    arm_biquad_cascade_df2T_init_f32(&S[i], 1, m_coef, m_buffer[i]);
  }

  m_err = ERR_OK;
  return true;

error_return:
  end();
  return false;
}

void IIRClass::end()
{
  for (int i = 0; i < m_channel; i++) {
    delete m_ringbuff[i];
    m_ringbuff[i] = NULL;
  }
  delete m_tmpInBuff;
  m_tmpInBuff =NULL;
  delete m_tmpOutBuff;
  m_tmpOutBuff =NULL;
  if (m_output == Interleave) {
    delete m_InterleaveBuff;
    m_InterleaveBuff = NULL;
  }
  m_err = ERR_OK;
}

bool IIRClass::create_coef(filterType_t type, int cutoff, float q)
{
  float w,k0,k1,a0,a1,a2,b0,b1,b2;

  w = 2.0f * PI * cutoff / m_fs;

  a1 = -2.0f * cos(w);

  switch(type){
  case (TYPE_LPF):
  case (TYPE_HPF):
    k0 = sin(w) / (2.0f * q);

    a0 =  1.0f + k0;
    a2 =  1.0f - k0;

    break;
  case (TYPE_BPF):
  case (TYPE_BEF):
    k0 = sin(w) * sinh(log(2.0f) / 2.0 * q * w / sin(w));
    a0 =  1.0f + k0;
    a2 =  1.0f - k0;
    break;
  default:
    return false;
  }

  switch(type){
  case TYPE_LPF:
    k1 = 1.0f - cos(w);
    b0 = k1 / 2.0f;
    b1 = k1;
    b2 = k1 / 2.0f;
    break;
  case TYPE_HPF:
    k1 = 1.0f + cos(w);
    b0 = k1 / 2.0f;
    b1 = -k1;
    b2 = k1 / 2.0f;
    break;
  case TYPE_BPF:
    b0 =  k0;
    b1 =  0.0f;
    b2 = -k0;
    break;
  case TYPE_BEF:
    b0 =  1.0f;
    b1 = -2.0f * cos(w);
    b2 =  1.0f;
    break;
  default:
    return false;
  }

  m_coef[0] = b0/a0;
  m_coef[1] = b1/a0;
  m_coef[2] = b2/a0;
  m_coef[3] = -(a1/a0);
  m_coef[4] = -(a2/a0);

  return true;
}

bool IIRClass::put(q15_t* pSrc, int sample)
{
  /* Ringbuf size check */
  for (int i = 0; i < m_channel; i++) {
    if (sample > m_ringbuff[i]->remain()) {
      m_err = ERR_BUF_FULL;
      return false;
    }
  }

  if (m_channel == 1) {
    /* the faster optimization */
    m_ringbuff[0]->put((q15_t*)pSrc, sample);
  } else {
    for (int i = 0; i < m_channel; i++) {
      m_ringbuff[i]->put(pSrc, sample, m_channel, i);
    }
  }

  m_err = ERR_OK;
  return  true;
}

bool IIRClass::empty(int channel)
{
  if (channel >= m_channel) {
    m_err = ERR_CH_NUM;
    return true;
  }

  return (m_ringbuff[channel]->stored() < m_framesize);
}

int IIRClass::get(q15_t* pDst, int channel)
{
  if (m_output == Interleave) {
    m_err = ERR_FORMAT;
    return ERR_FORMAT;
  }
  if (channel >= m_channel) {
    m_err = ERR_CH_NUM;
    return ERR_CH_NUM;
  }
  if (empty(channel)) {
    m_err = ERR_OK;
    return 0;
  }

  /* Read from the ring buffer */
  m_ringbuff[channel]->get(m_tmpInBuff, m_framesize);

  arm_biquad_cascade_df2T_f32(&S[channel], m_tmpInBuff, m_tmpOutBuff, m_framesize);
  arm_float_to_q15(m_tmpOutBuff, pDst, m_framesize);

  m_err = ERR_OK;
  return m_framesize;
}

int IIRClass::get(q15_t* pDst)
{
  if (m_output == Planar) {
    m_err = ERR_FORMAT;
    return ERR_FORMAT;
  }
  for (int i = 0; i < m_channel; i++) {
    if (empty(i)) {
      m_err = ERR_OK;
      return 0;
    }
  }

  /* Read from the ring buffer */
  for (int i = 0; i < m_channel; i++) {
    m_ringbuff[i]->get(m_tmpInBuff, m_framesize);

    arm_biquad_cascade_df2T_f32(&S[i], m_tmpInBuff, m_tmpOutBuff, m_framesize);
    arm_float_to_q15(m_tmpOutBuff, m_InterleaveBuff, m_framesize);

    for (int j = 0; j < m_framesize; j++) {
      *(pDst + (j * m_channel) + i) = *(m_InterleaveBuff + j);
    }
  }

  m_err = ERR_OK;
  return m_framesize;
}
