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
    *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <Audio.h>
#include <SD.h>

//#include <fcntl.h>
#include <arch/board/board.h>

AudioClass *theAudio;
File myFile;

void setup()
{
  /* Initialize SD Card */
  while (!SD.begin()) {
    ; /* wait until SD card is mounted. */
  }

  theAudio = AudioClass::getInstance();
  theAudio->begin();

  puts("initialization Audio Library");

  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);
  theAudio->initRecorder(AS_CODECTYPE_WAV,"/mnt/sd0/BIN",AS_SAMPLINGRATE_48000,AS_CHANNEL_STEREO);
  puts("Init Recorder!");

  myFile = SD.open("Sound.wav", FILE_WRITE);
  puts("Open!");

  theAudio->writeWavHeader(myFile);
  puts("Write Header!");

  theAudio->startRecorder();
  puts("Start Rec!");

}

void loop() {
  // put your main code here, to run repeatedly:

  // for Example, Chack Bottom

  static int cnt = 0;

  if (cnt>400)
    {
      puts("End Recording");
      theAudio->stopRecorder();
      theAudio->closeOutputFile(myFile);
      exit(1);
    }

  err_t err = theAudio->readFrames(myFile);

  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("File End! =%d\n",err);
      sleep(1);
      theAudio->stopRecorder();
      theAudio->closeOutputFile(myFile);
      exit(1);
    }

  usleep(1);

  cnt++;
}



