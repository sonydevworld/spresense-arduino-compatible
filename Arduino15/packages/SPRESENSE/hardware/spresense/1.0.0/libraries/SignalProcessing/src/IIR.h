/*
 *  IIR.h - IIR(biquad cascade) Library Header
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

#ifndef _IIR_H_
#define _IIR_H_

/* Use CMSIS library */
#define ARM_MATH_CM4
#define __FPU_PRESENT 1U
#include <cmsis/arm_math.h>

#include "RingBuff.h"

/*------------------------------------------------------------------*/
/* Configurations                                                   */
/*------------------------------------------------------------------*/
/* Select Data Bit length */

#define BITLEN 16
//#define BITLEN 32

/* Max sample of frame */
#define FRAMSIZE 768

/* Number of channels */
#define MAX_CHANNEL_NUM 1
//#define MAX_CHANNEL_NUM 2
//#define MAX_CHANNEL_NUM 4

#define INPUT_BUFFER (MAX_CHANNEL_NUM*sizeof(q15_t)*FRAMSIZE*3)

/*------------------------------------------------------------------*/
/* Type Definition                                                  */
/*------------------------------------------------------------------*/
/* FILTER TYPE */
typedef enum e_filterType {
  TYPE_LPF,
  TYPE_HPF,
  TYPE_BPF,
  TYPE_BEF
} filterType_t;


/*------------------------------------------------------------------*/
/* Input buffer                                                      */
/*------------------------------------------------------------------*/
class IIRClass
{
public:
  bool begin(filterType_t type, int channel, int cutoff, float q);
  bool put(q15_t* pSrc, int size);
  int  get(q15_t* pDst, int channel);
  void end(){}
  bool empty(int channel);

private:

  int m_channel;

  arm_biquad_cascade_df2T_instance_f32 S[MAX_CHANNEL_NUM];

  float32_t coef[5];
  float32_t buffer[4];

  /* Temporary buffer */
  float tmpInBuf[FRAMSIZE];
  float tmpOutBuf[FRAMSIZE];

  bool create_coef(filterType_t, int cutoff, float q);

};

#endif /*_FFT_H_*/
