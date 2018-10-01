/*
 *  recorder.ino - Recorder example application
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
#include <SD.h>

AudioClass *theAudio;

File myFile;

/**
 * @brief Setup recording of mp3 stream to file
 *
 * Select input device as microphone <br>
 * Initialize filetype to stereo mp3 with 48 Kb/s sampling rate <br>
 * Open "Sound.mp3" file in write mode
 */
void setup()
{
 /* Initialize SD Card */
  while (!SD.begin()) {
    ; /* wait until SD card is mounted. */
  }
 
  theAudio = AudioClass::getInstance();
  theAudio->begin();

  puts("initialization Audio Library");

  /* Select input device as microphone */
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);

  /*
   * Initialize filetype to stereo mp3 with 48 Kb/s sampling rate
   * Search for MP3 codec in "/mnt/sd0/BIN" directory
   */
  theAudio->initRecorder(AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_48000, AS_CHANNEL_STEREO);
  puts("Init Recorder!");

  /* Open file for data write on SD card */
  myFile = SD.open("Sound.mp3", FILE_WRITE);
  /* Verify file open */
  if (!myFile)
    {
      printf("File open error\n");
      exit(1);
    }
  puts("Open!");

  puts("Rec!");

  theAudio->startRecorder();
}

/**
 * @brief Record given frame number
 */
void loop() {

  static int cnt = 0;

  /* recording end condition */
  if (cnt > 400)
    {
      puts("End Recording");
      theAudio->stopRecorder();
      theAudio->closeOutputFile(myFile);
      myFile.close();
      exit(1);
    }

  /* Read frames to record in file */
  err_t err = theAudio->readFrames(myFile);

  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("File End! =%d\n",err);
      sleep(1);
      theAudio->stopRecorder();
      theAudio->closeOutputFile(myFile);
      myFile.close();
      exit(1);
    }

  usleep(1);

  cnt++;
}
