/*
 *  IIR.cpp - IIR(biquad cascade) Library
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

#include "IIR.h"

RingBuff ringbuf[MAX_CHANNEL_NUM](INPUT_BUFFER);

bool IIRClass::begin(filterType_t type, int channel, int cutoff, float q)
{
  if(channel > MAX_CHANNEL_NUM) return false;

  m_channel = channel;

  if(create_coef(type, cutoff, q) == false){
    return false;
  }

  for (int i = 0; i < channel; i++) {
    arm_biquad_cascade_df2T_init_f32(&S[i],1,coef,buffer);
  }

  return true;
}

bool IIRClass::create_coef(filterType_t type, int cutoff, float q)
{
  float w,k0,k1,a0,a1,a2,b0,b1,b2;

  w = 2.0f * PI * cutoff / 48000;

  a1 = -2.0f * cos(w);

  switch(type){
  case (TYPE_LPF):
  case (TYPE_HPF):
    k0 = sin(w) / (2.0f * q);
    k1 = 1.0f - cos(w);

    a0 =  1.0f + k0;
    a2 =  1.0f - k0;
    b0 =  k1 / 2.0f;
    b2 = (k0) / 2.0f;

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
    b1 =  k1;
    break;
  case TYPE_HPF:
    b1 =  -k1;
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

  coef[0] = b0/a0;
  coef[1] = b1/a0;
  coef[2] = b2/a0;
  coef[3] = -(a1/a0);
  coef[4] = -(a2/a0);

  return true;
}


bool IIRClass::put(q15_t* pSrc, int sample)
{
  /* Ringbuf size check */
  if(m_channel > MAX_CHANNEL_NUM) return false;
  if(sample > ringbuf[0].remain()) return false;

  if (m_channel == 1) {
    /* the faster optimization */
    ringbuf[0].put((q15_t*)pSrc, sample);
  } else {
    for (int i = 0; i < m_channel; i++) {
      ringbuf[i].put(pSrc, sample, m_channel, i);
    }
  }
  return  true;
}

bool IIRClass::empty(int channel)
{
   return (ringbuf[channel].stored() < FRAMSIZE);
}

int IIRClass::get(q15_t* pDst, int channel)
{

  if(channel >= m_channel) return false;
  if (ringbuf[channel].stored() < FRAMSIZE) return 0;

  /* Read from the ring buffer */
  ringbuf[channel].get(tmpInBuf, FRAMSIZE);

  arm_biquad_cascade_df2T_f32(&S[channel], tmpInBuf, tmpOutBuf, FRAMSIZE);
  arm_float_to_q15(tmpOutBuf, pDst, FRAMSIZE);

  return FRAMSIZE;
}
