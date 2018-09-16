/*
 *  sound_fft.ino - PCM capture (Sound) and FFT example application
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

#include <Audio.h>

#include <SDHCI.h>

AudioClass *theAudio;
SDClass theSD;
File myFile;


const int32_t recoding_frames = 400;
//const int32_t buffer_size = 6144;
const int32_t buffer_size = 768*2; 
char buffer[buffer_size];
float buffer_f[buffer_size/2];

#include "dsp_rpc.h"

#define MAX_BLOCKSIZE   128
//#define MAX_BLOCKSIZE   256
#define DSPMATH_LIBFILE "/mnt/sd0/BIN/DSPFFT"

float32_t testOutput[MAX_BLOCKSIZE/2];

/**
 *  @brief Setup audio device to capture PCM stream
 *
 *  Select input device as analog microphone, AMIC  <br>
 *  Set PCM capture sapling rate parameters to 48 kb/s <br>
 *  Set channel number 4 to capture audio from 4 microphones simultaneously <br>
 *  System directory "/mnt/sd0/BIN" will be searched for PCM codec
 */
void setup()
{
  theAudio = AudioClass::getInstance();

  theAudio->begin();

  myFile = theSD.open("Sound.pcm", FILE_WRITE);

  puts("initialization Audio Library");

  load_library(DSPMATH_LIBFILE);
  init_fft_f32( MAX_BLOCKSIZE, 0, 1);

  /* Select input device as AMIC */
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);

  /*
   * Set PCM capture sapling rate parameters to 48 kb/s. Set channel number 4
   * Search for PCM codec in "/mnt/sd0/BIN" directory
   */
//  theAudio->initRecorder(AS_CODECTYPE_PCM, "/mnt/sd0/BIN", AS_SAMPLINGRATE_48000, AS_CHANNEL_4CH);
  theAudio->initRecorder(AS_CODECTYPE_PCM, "/mnt/sd0/BIN", AS_SAMPLINGRATE_48000, AS_CHANNEL_MONO);
  puts("Init Recorder!");

  puts("Rec!");
  theAudio->startRecorder();
}

/**
 * @brief Capture frames of PCM data into buffer
 */
void loop() {

  static int cnt = 0;
  uint32_t read_size;

  /* recording end condition */
  if (cnt > recoding_frames)
    {
      puts("End Recording");
      theAudio->stopRecorder();
  myFile.close();
      exit(1);
    }

  /* Read frames to record in buffer */
  int err = theAudio->readFrames(buffer, buffer_size, &read_size);

  if (err != AUDIOLIB_ECODE_OK && err != AUDIOLIB_ECODE_INSUFFICIENT_BUFFER_AREA)
    {
      printf("Error End! =%d\n",err);
      sleep(1);
      theAudio->stopRecorder();
      exit(1);
    }

  /* The actual signal processing will be coding here.
     For example, prints capture data. */

  if (read_size != 0)
  {
    int16_t* pt = (int16_t*)buffer;
    for(int i=0 ; i<768;i++){
      buffer_f[i] = ((float)*pt)/0x7FFF;
      pt++;
   }
   
  /* Transform input a[n] from time domain to frequency domain A[k] */

// int ret = myFile.write((uint8_t*)buffer, 768*2);  // for 16bit data
   int ret = myFile.write((uint8_t*)buffer_f, 768*4);

    float32_t* in  = buffer_f;
    float32_t* out = testOutput;
    rev_fft_f32(in, out); // For parallel execute

//  exec_fft_f32(in, out);
    send_fft_f32(in, out);
  
    for(int i=0;i<(MAX_BLOCKSIZE/2/2);i++){
      printf("%2.8f\n",testOutput[i]);
    }
  }

//  volatile int i;  
//  for(i=0; i<100000;i++);

  cnt++;

}
