/*
 *  FFT.h - FFT Library
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

#ifndef _FFT_H_
#define _FFT_H_

/* Use CMSIS library */
#define ARM_MATH_CM4
#define __FPU_PRESENT 1U
#include <cmsis/arm_math.h>

#include "RingBuff.h"

/*------------------------------------------------------------------*/
/* Type Definition                                                  */
/*------------------------------------------------------------------*/
/* WINDOW TYPE */
typedef enum e_windowType {
  WindowHamming,
  WindowHanning,
  WindowFlattop,
  WindowRectangle
} windowType_t;

/*------------------------------------------------------------------*/
/* Input buffer                                                      */
/*------------------------------------------------------------------*/
template <int MAX_CHNUM, int FFTLEN> class FFTClass
{
public:
  void begin(){
      begin(WindowHamming, MAX_CHNUM, (FFTLEN / 2));
  }

  bool begin(windowType_t type, int channel, int overlap){
    if (channel > MAX_CHNUM) return false;
    if (overlap > (FFTLEN / 2)) return false;

    m_overlap = overlap;
    m_channel = channel;

    clear();
    create_coef(type);
    if (!fft_init()) {
       return false;
    }

    for(int i = 0; i < MAX_CHNUM; i++) {
      ringbuf_fft[i] = new RingBuff(MAX_CHNUM * FFTLEN * sizeof(q15_t));
    }

    return true;
  }

  bool put(q15_t* pSrc, int sample) {
    /* Ringbuf size check */
    if(m_channel > MAX_CHNUM) return false;
    if(sample > ringbuf_fft[0]->remain()) return false;

    if (m_channel == 1) {
      /* the faster optimization */
      ringbuf_fft[0]->put((q15_t*)pSrc, sample);
    } else {
      for (int i = 0; i < m_channel; i++) {
        ringbuf_fft[i]->put(pSrc, sample, m_channel, i);
      }
    }
    return  true;
  }

  int  get_raw(float* out, int channel) {
    return get_raw(out, channel, true);
  }

  int  get(float* out, int channel) {
    return get_raw(out, channel, false);
  }

  void clear() {
    for (int i = 0; i < MAX_CHNUM; i++) {
      memset(tmpInBuf[i], 0, FFTLEN);
    }
  }

  void end(){}


  bool empty(int channel){
    return (ringbuf_fft[channel]->stored() < FFTLEN);
  }

private:

  RingBuff* ringbuf_fft[MAX_CHNUM];

  int m_channel;
  int m_overlap;
  arm_rfft_fast_instance_f32 S;

  /* Temporary buffer */
  float tmpInBuf[MAX_CHNUM][FFTLEN];
  float coef[FFTLEN];
  float tmpOutBuf[FFTLEN];

  void create_coef(windowType_t type) {
    for (int i = 0; i < FFTLEN / 2; i++) {
      if (type == WindowHamming) {
        coef[i] = 0.54f - (0.46f * arm_cos_f32(2 * PI * (float)i / (FFTLEN - 1)));
      } else if (type == WindowHanning) {
        coef[i] = 0.54f - (1.0f * arm_cos_f32(2 * PI * (float)i / (FFTLEN - 1)));
      } else if (type == WindowFlattop) {
        coef[i] = 0.21557895f - (0.41663158f  * arm_cos_f32(2 * PI * (float)i / (FFTLEN - 1)))
                              + (0.277263158f * arm_cos_f32(4 * PI * (float)i / (FFTLEN - 1)))
                              - (0.083578947f * arm_cos_f32(6 * PI * (float)i / (FFTLEN - 1)))
                              + (0.006947368f * arm_cos_f32(8 * PI * (float)i / (FFTLEN - 1)));
      } else {
        coef[i] = 1;
      }
      coef[FFTLEN -1 - i] = coef[i];
    }
  }

  bool fft_init(){
    switch (FFTLEN){
      case 32:
        arm_rfft_32_fast_init_f32(&S);
        break;
      case 64:
        arm_rfft_64_fast_init_f32(&S);
        break;
      case 128:
        arm_rfft_128_fast_init_f32(&S);
        break;
      case 256:
        arm_rfft_256_fast_init_f32(&S);
        break;
      case 512:
        arm_rfft_512_fast_init_f32(&S);
        break;
      case 1024:
        arm_rfft_1024_fast_init_f32(&S);
        break;
      case 2048:
        arm_rfft_2048_fast_init_f32(&S);
        break;
      case 4096:
        arm_rfft_4096_fast_init_f32(&S);
        break;
      default:
        puts("error!");
        return false;
        break;
    }
    return true;
  }

  void fft(float *pSrc, float *pDst) {
    arm_rfft_fast_f32(&S, pSrc, pDst, 0);
  }

  void fft_amp(float *pSrc, float *pDst) {
    /* calculation */
    arm_rfft_fast_f32(&S, pSrc, tmpOutBuf, 0);
    arm_cmplx_mag_f32(tmpOutBuf, pDst, FFTLEN / 2);
  }

  int get_raw(float* out, int channel, int raw) {
    static float tmpFft[FFTLEN];

    if(channel >= m_channel) return false;
    if (ringbuf_fft[channel]->stored() < FFTLEN) return 0;

    for (int i=0;i<m_overlap;i++) {
      tmpInBuf[channel][i] = tmpInBuf[channel][FFTLEN - m_overlap + i];
    }

    /* Read from the ring buffer */
    ringbuf_fft[channel]->get(&tmpInBuf[channel][m_overlap], FFTLEN - m_overlap);

    for (int i = 0; i < FFTLEN; i++) {
      tmpFft[i] = tmpInBuf[channel][i] * coef[i];
    }

    if(raw){
      /* Calculate only FFT */
      fft(tmpFft, out);
    }else{
      /* Calculate FFT for convert to amplitude */
      fft_amp(tmpFft, out);
    }
    return (FFTLEN - m_overlap);
  }


};

#endif /*_FFT_H_*/
