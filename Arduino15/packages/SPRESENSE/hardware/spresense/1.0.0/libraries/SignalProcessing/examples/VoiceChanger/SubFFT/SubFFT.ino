/*
 *  SubFFT.ino - FFT Example with Audio (voice changer)
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

#include <MP.h>

#include "FFT.h"

/*-----------------------------------------------------------------*/
/*
 * FFT parameters
 */
/* Select FFT length */

//#define FFT_LEN 32
//#define FFT_LEN 64
//#define FFT_LEN 128
//#define FFT_LEN 256
//#define FFT_LEN 512
#define FFT_LEN 1024
//#define FFT_LEN 2048
//#define FFT_LEN 4096

/* Number of channels*/
#define MAX_CHANNEL_NUM 1
//#define MAX_CHANNEL_NUM 2
//#define MAX_CHANNEL_NUM 4

FFTClass<MAX_CHANNEL_NUM, FFT_LEN> FFT;

arm_rfft_fast_instance_f32 iS;
arm_biquad_cascade_df2T_instance_f32 bS;

float32_t lpf_coef[5];
float32_t lpf_buffer[4];

/* Allocate the larger heap size than default */

USER_HEAP_SIZE(64 * 1024);

/* MultiCore definitions */

struct Request {
  void *buffer;
  int  sample;
  int  pitch_shift;
};

struct Result {
  void *buffer;
  int  sample;
};

void create_coef(int cutoff, float q)
{
  float w,k0,k1,a0,a1,a2,b0,b1,b2;

  w = 2.0f * PI * cutoff / 48000;

  a1 = -2.0f * cos(w);
  k0 = sin(w) / (2.0f * q);
  k1 = 1.0f - cos(w);

  a0 =  1.0f + k0;
  a2 =  1.0f - k0;
  b0 =  k1 / 2.0f;
  b1 =  k1;
  b2 = (k0) / 2.0f;

  lpf_coef[0] = b0/a0;
  lpf_coef[1] = b1/a0;
  lpf_coef[2] = b2/a0;
  lpf_coef[3] = -(a1/a0);
  lpf_coef[4] = -(a2/a0);
}

void setup()
{
  int ret = 0;

  /* Initialize MP library */
  ret = MP.begin();
  if (ret < 0) {
    errorLoop(2);
  }

  /* receive with non-blocking */
//  MP.RecvTimeout(MP_RECV_POLLING);
  MP.RecvTimeout(100000);

  FFT.begin(WindowRectangle,MAX_CHANNEL_NUM,0);
  create_coef(1000, 1);

  arm_rfft_1024_fast_init_f32(&iS);
  arm_biquad_cascade_df2T_init_f32(&bS,1,lpf_coef,lpf_buffer);
}

#define RESULT_SIZE 4
#define MAX_SHIFT 20
void loop()
{
  int      ret;
  int8_t   sndid = 10; /* user-defined msgid */
  int8_t   rcvid;
  Request *request;
  Result   result[RESULT_SIZE];

  static float pTmp[(FFT_LEN+(MAX_SHIFT*2))*2];
  static float pDst[FFT_LEN];
  static float pLpfTmp[FFT_LEN];
  static q15_t pOut[RESULT_SIZE][FFT_LEN];
  static int pos=0;
  static int pitch_shift = 0;

  /* Receive PCM captured buffer from MainCore */
  ret = MP.Recv(&rcvid, &request);
  if (ret >= 0) {
      FFT.put((q15_t*)request->buffer,request->sample);
      pitch_shift = request->pitch_shift;
  }
  while(!FFT.empty(0)){
    for (int i = 0; i < MAX_CHANNEL_NUM; i++) {

      int cnt = FFT.get_raw(&pTmp[(MAX_SHIFT+pitch_shift)*2],i);

      if(pitch_shift>0){
        FFT.get_raw(&pTmp[(pitch_shift)*2],i);
        memset(pTmp,0,pitch_shift*2);
        arm_rfft_fast_f32(&iS, &pTmp[0], pDst, 1);
      }else{
        FFT.get_raw(&pTmp[(MAX_SHIFT-pitch_shift)*2],i);
        memset((pTmp+(FFT_LEN-MAX_SHIFT)),0,MAX_SHIFT*2);
        arm_rfft_fast_f32(&iS, &pTmp[MAX_SHIFT*2], pDst, 1);
      }

      arm_biquad_cascade_df2T_f32(&bS, pDst, pLpfTmp, FFT_LEN);
      arm_float_to_q15(pLpfTmp,&pOut[pos][0],FFT_LEN);

      result[pos].buffer = MP.Virt2Phys(&pOut[pos][0]);
      result[pos].sample = FFT_LEN;

      ret = MP.Send(sndid, &result[pos],0);
      pos = (pos+1)%RESULT_SIZE;
      if (ret < 0) {
        errorLoop(1);
      }
    }
  }
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
