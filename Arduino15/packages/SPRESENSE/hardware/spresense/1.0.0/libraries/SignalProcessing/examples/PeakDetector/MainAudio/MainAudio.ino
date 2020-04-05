/*
 *  MainAudio.ino - FFT Example with Audio (Peak detector)
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
#include <Audio.h>

AudioClass *theAudio;

/* Select mic channel number */
//const int mic_channel_num = 1;
//const int mic_channel_num = 2;
const int mic_channel_num = 4;

const int subcore = 1;

struct Request {
  void *buffer;
  int  sample;
  int  channel;
};

struct Result {
  float peak[mic_channel_num];
  int  channel;
};

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Init Audio Library");
  theAudio = AudioClass::getInstance();
  theAudio->begin();

  Serial.println("Init Audio Recorder");
  /* Select input device as AMIC */
  //theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC, 210);
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);

  /* Set PCM capture */
  uint8_t channel;
  switch (mic_channel_num) {
  case 1: channel = AS_CHANNEL_MONO;   break;
  case 2: channel = AS_CHANNEL_STEREO; break;
  case 4: channel = AS_CHANNEL_4CH;    break;
  }
  theAudio->initRecorder(AS_CODECTYPE_PCM, "/mnt/sd0/BIN", AS_SAMPLINGRATE_48000, channel);

  /* Launch SubCore */
  int ret = MP.begin(subcore);
  if (ret < 0) {
    printf("MP.begin error = %d\n", ret);
  }
  /* receive with non-blocking */
  MP.RecvTimeout(1);

  Serial.println("Rec start!");
  theAudio->startRecorder();
}

void loop()
{
  int8_t   sndid = 100; /* user-defined msgid */
  int8_t   rcvid = 0;
  Request  request;
  Result*  result;

  static const int32_t buffer_sample = 768 * mic_channel_num;
  static const int32_t buffer_size = buffer_sample * sizeof(int16_t);
  static char  buffer[buffer_size];
  uint32_t read_size;

  /* Read frames to record in buffer */
  int err = theAudio->readFrames(buffer, buffer_size, &read_size);

  if (err != AUDIOLIB_ECODE_OK && err != AUDIOLIB_ECODE_INSUFFICIENT_BUFFER_AREA) {
    printf("Error err = %d\n", err);
    sleep(1);
    theAudio->stopRecorder();
    exit(1);
  }
  if ((read_size != 0) && (read_size == buffer_size)) {
    request.buffer   = buffer;
    request.sample = buffer_sample / mic_channel_num;
    request.channel  = mic_channel_num;
    MP.Send(sndid, &request, subcore);
  } else {
    /* Receive detector results from SubCore */
    int ret = MP.Recv(&rcvid, &result, subcore);
    if (ret >= 0) {
      for (int i=0;i<mic_channel_num;i++) {
        printf("%8.3f, ", result->peak[i]);
      }
      printf("\n");
    }
  }
}
