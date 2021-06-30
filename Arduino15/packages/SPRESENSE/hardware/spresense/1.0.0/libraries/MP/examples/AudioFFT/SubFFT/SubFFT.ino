/*
 *  SubFFT.ino - MP Example for Audio FFT 
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

#ifndef SUBCORE
#error "Core selection is wrong!!"
#endif

#include <MP.h>

/* Use CMSIS library */
#define ARM_MATH_CM4
#define __FPU_PRESENT 1U
#include <arm_math.h>

#include "RingBuff.h"

/* Select FFT length */

//#define FFTLEN 32
//#define FFTLEN 64
//#define FFTLEN 128
//#define FFTLEN 256
//#define FFTLEN 512
#define FFTLEN 1024
//#define FFTLEN 2048
//#define FFTLEN 4096

const int g_channel = 4;

/* Ring buffer */

#define INPUT_BUFFER (1024 * 4)
RingBuff ringbuf[g_channel] = {
  RingBuff(INPUT_BUFFER),
  RingBuff(INPUT_BUFFER),
  RingBuff(INPUT_BUFFER),
  RingBuff(INPUT_BUFFER)
};

/* Allocate the larger heap size than default */

USER_HEAP_SIZE(64 * 1024);

/* Temporary buffer */

float pSrc[FFTLEN];
float pDst[FFTLEN];
float tmpBuf[FFTLEN];

/* MultiCore definitions */

struct Capture {
  void *buff;
  int  sample;
  int  chnum;
};

void setup()
{
  int ret = 0;

  /* Initialize MP library */
  ret = MP.begin();
  if (ret < 0) {
    errorLoop(2);
  }
  /* receive with non-blocking */
  MP.RecvTimeout(MP_RECV_POLLING);
}

void loop()
{
  int      ret;
  int8_t   msgid;
  Capture *capture;

  /* Receive PCM captured buffer from MainCore */
  ret = MP.Recv(&msgid, &capture);
  if (ret >= 0) {
    if (capture->chnum == 1) {
      /* the faster optimization */
      ringbuf[0].put((q15_t*)capture->buff, capture->sample);
    } else {
      int i;
      for (i = 0; i < capture->chnum; i++) {
        ringbuf[i].put((q15_t*)capture->buff, capture->sample, capture->chnum, i);
      }
    }
  }

  while (ringbuf[0].stored() >= FFTLEN) {
    fft_processing(capture->chnum);
  }
}

void fft_processing(int chnum)
{
  int i;
  float peakFs[4] = {0.0f, 0.0f, 0.0f, 0.0f};

  for (i = 0; i < chnum; i++) {
    /* Read from the ring buffer */
    ringbuf[i].get(pSrc, FFTLEN);

    /* Calculate FFT */
    fft(pSrc, pDst, FFTLEN);

    /* Peak */
    peakFs[i] = get_peak_frequency(pDst, FFTLEN);
  }

  printf("24000 %8.3f %8.3f %8.3f %8.3f\n",
         peakFs[0], peakFs[1], peakFs[2], peakFs[3]);
}

void fft(float *pSrc, float *pDst, int fftLen)
{
  arm_rfft_fast_instance_f32 S;

#if (FFTLEN == 32)
  arm_rfft_32_fast_init_f32(&S);
#elif (FFTLEN == 64)
  arm_rfft_64_fast_init_f32(&S);
#elif (FFTLEN == 128)
  arm_rfft_128_fast_init_f32(&S);
#elif (FFTLEN == 256)
  arm_rfft_256_fast_init_f32(&S);
#elif (FFTLEN == 512)
  arm_rfft_512_fast_init_f32(&S);
#elif (FFTLEN == 1024)
  arm_rfft_1024_fast_init_f32(&S);
#elif (FFTLEN == 2048)
  arm_rfft_2048_fast_init_f32(&S);
#elif (FFTLEN == 4096)
  arm_rfft_4096_fast_init_f32(&S);
#endif

  /* calculation */
  arm_rfft_fast_f32(&S, pSrc, tmpBuf, 0);

  arm_cmplx_mag_f32(&tmpBuf[2], &pDst[1], fftLen / 2 - 1);
  pDst[0] = tmpBuf[0];
  pDst[fftLen / 2] = tmpBuf[1];
}

float get_peak_frequency(float *pData, int fftLen)
{
  float g_fs = 48000.0f;
  uint32_t index;
  float maxValue;
  float delta;
  float peakFs;

  arm_max_f32(pData, fftLen / 2, &maxValue, &index);

  delta = 0.5 * (pData[index - 1] - pData[index + 1])
    / (pData[index - 1] + pData[index + 1] - (2.0f * pData[index]));
  peakFs = (index + delta) * g_fs / (fftLen - 1);

  return peakFs;
}

void errorLoop(int num)
{
  int i;

  while (1) {
    for (i = 0; i < num; i++) {
      ledOn(LED0);
      delay(300);
      ledOff(LED0);
      delay(300);
    }
    delay(1000);
  }
}
