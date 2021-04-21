/*
 *  SubFFT.ino - FFT Example with Audio (peak detector)
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
//#define MAX_CHANNEL_NUM 1
//#define MAX_CHANNEL_NUM 2
#define MAX_CHANNEL_NUM 4

FFTClass<MAX_CHANNEL_NUM, FFT_LEN> FFT;

/* Allocate the larger heap size than default */

USER_HEAP_SIZE(64 * 1024);

/* MultiCore definitions */

struct Request {
  void *buffer;
  int  sample;
  int  chnum;
};

struct Result {
  Result() {
    clear();
  }

  float peak[MAX_CHANNEL_NUM];
  int  channel;

  void clear() {
    for (int i = 0; i < MAX_CHANNEL_NUM; i++) {
      peak[i] = 0;
    }
  }
};

void setup()
{
  /* Initialize MP library */
  int ret = MP.begin();
  if (ret < 0) {
    errorLoop(2);
  }

  /* receive with non-blocking */
  MP.RecvTimeout(MP_RECV_POLLING);

  FFT.begin();
}

#define RESULT_SIZE 4
void loop()
{
  int      ret;
  int8_t   sndid = 10; /* user-defined msgid */
  int8_t   rcvid;
  Request *request;
  static Result result[RESULT_SIZE];
  static int pos = 0;

  static float pDst[FFT_LEN / 2];

  /* Receive PCM captured buffer from MainCore */
  ret = MP.Recv(&rcvid, &request);
  if (ret >= 0) {
    FFT.put((q15_t*)request->buffer, request->sample);
  }

  while (!FFT.empty(0)) {
    result[pos].clear();
    result[pos].channel = MAX_CHANNEL_NUM;
    for (int i = 0; i < MAX_CHANNEL_NUM; i++) {
      FFT.get(pDst, i);
      result[pos].peak[i] = get_peak_frequency(pDst, FFT_LEN);
//    printf("%8.3f, ", result[pos].peak[i]);
    }
//  printf("\n");

    ret = MP.Send(sndid, &result[pos], 0);
    pos = (pos + 1) % RESULT_SIZE;
    if (ret < 0) {
      errorLoop(1);
    }
  }
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
