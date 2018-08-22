/*
 *  pcm_capture.ino - PCM capture example application
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

AudioClass *theAudio;

const int32_t recoding_frames = 400;
const int32_t buffer_size = 6144;
char buffer[buffer_size];

/**
 *  @brief Setup audio device to capture PCM stream
 *
 *  Select input device as microphone <br>
 *  Set PCM capture sapling rate parameters to 48 kb/s <br>
 *  Set channel number 4 to capture audio from 4 microphones simultaneously <br>
 *  System directory "/mnt/sd0/BIN" will be searched for PCM codec
 */
void setup()
{
  theAudio = AudioClass::getInstance();

  theAudio->begin();

  puts("initialization Audio Library");

  /* Select input device as microphone */
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);

  /*
   * Set PCM capture sapling rate parameters to 48 kb/s. Set channel number 4
   * Search for PCM codec in "/mnt/sd0/BIN" directory
   */
  theAudio->initRecorder(AS_CODECTYPE_PCM, "/mnt/sd0/BIN", AS_SAMPLINGRATE_48000, AS_CHANNEL_4CH);
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
      exit(1);
    }

  /* Read frames to record in buffer */
  err_t err = theAudio->readFrames(buffer, buffer_size, &read_size);

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
    printf("Record Data: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
  }

  /* The actual signal processing will be done here.
     Adjust the waiting time as necessary. */

  volatile int i;  
  for(i=0; i<100000;i++);

  cnt++;

}
