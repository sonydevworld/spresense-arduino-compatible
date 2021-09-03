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
#include "IIR.h"

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

/* Parameters */
const int   g_channel = MAX_CHANNEL_NUM; /* Number of channels */
const int   g_cutoff  = 1000; /* Cutoff frequency */
const float g_Q       = sqrt(0.5); /* Q Value */
const int   g_sample  = 1024; /* Number of channels */

const int   g_max_shift = 30; /* Pitch shift value */

const int   g_result_size = 4; /* Result buffer size */

FFTClass<MAX_CHANNEL_NUM, FFT_LEN> FFT;
IIRClass LPF;

arm_rfft_fast_instance_f32 iS;
arm_biquad_cascade_df2T_instance_f32 bS;

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

void setup()
{
  int ret = 0;

  /* Initialize MP library */
  ret = MP.begin();
  if (ret < 0) {
    errorLoop(2);
  }

  /* receive with non-blocking */
  MP.RecvTimeout(100000);

  /* begin FFT */
  FFT.begin(WindowRectangle,MAX_CHANNEL_NUM,0);

  /* begin LPF */
  if(!LPF.begin(TYPE_LPF, g_channel, g_cutoff, g_Q, g_sample)) {
    int err = LPF.getErrorCause();
    printf("error! %d\n", err);
    errorLoop(abs(err));
  }

  arm_rfft_1024_fast_init_f32(&iS);
}

void loop()
{
  int      ret;
  int8_t   sndid = 10; /* user-defined msgid */
  int8_t   rcvid;
  Request  *request;
  static Result result[g_result_size];

  static float pTmp[(FFT_LEN + g_max_shift) * 2];
  static float pDst[FFT_LEN];
  static q15_t pLpfTmp[FFT_LEN];
  static q15_t pLpfDst[FFT_LEN*2]; /* for stereo */
  static q15_t pOut[g_result_size][FFT_LEN*2];
  static int pos = 0;
  static int pitch_shift = 0;

  /* Receive PCM captured buffer from MainCore */
  ret = MP.Recv(&rcvid, &request);
  if (ret >= 0) {
      FFT.put((q15_t*)request->buffer,request->sample);
      pitch_shift = request->pitch_shift;
  }

  if ((pitch_shift < -g_max_shift) || (pitch_shift > g_max_shift)) {
    puts("Shift value error.");
    errorLoop(10);
  }

  while (!FFT.empty(0)) {
    for (int i = 0; i < g_channel; i++) {
      if (pitch_shift > 0) {
        memset(pTmp, 0, pitch_shift * 2);
        FFT.get_raw(&pTmp[(pitch_shift) * 2], i);
        arm_rfft_fast_f32(&iS, &pTmp[0], pDst, 1);
      } else {
        FFT.get_raw(&pTmp[0],i);
        memset(pTmp+(FFT_LEN), 0, abs(pitch_shift) * 2);
        arm_rfft_fast_f32(&iS, &pTmp[abs(pitch_shift) * 2], pDst, 1);
      }
      
      if (i == 0) {
        arm_float_to_q15(pDst, pLpfTmp, FFT_LEN);
        LPF.put(pLpfTmp, FFT_LEN);
        int cnt = LPF.get(pLpfDst, 0);
        printf("cnt=%d\n", cnt);
        for(int j = 0; j < cnt; j++) {
          pOut[pos][j * 2]     = pLpfDst[j];
          pOut[pos][j * 2 + 1] = 0;
        }

        result[pos].buffer = (void*)MP.Virt2Phys(&pOut[pos][0]);
        result[pos].sample = cnt;

        ret = MP.Send(sndid, &result[pos],0);
        pos = (pos + 1) % g_result_size;
        if (ret < 0) {
          errorLoop(11);
        }
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
