/*
 *  dual_players.ino - Dual source sound player example application
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

#include <SDHCI.h>
#include <Audio.h>

SDClass theSD;
AudioClass *theAudio;

File mainFile,subFile;

/**
 *  @brief Setup main audio player and sub audio player
 *
 *  Set output device to speaker <br>
 *  Open two stream files "Sound0.mp3" and "Sound1.mp3" <br>
 *  These should be in the root directory of the SD card. <br>
 *  Set main player to decode stereo mp3. Stream sample rate is set to "auto detect" <br>
 *  System directory "/mnt/sd0/BIN" will be searched for MP3 decoder (MP3DEC file) <br>
 *  This is the /BIN directory on the SD card. <br>
 *  Volume is set to -8.0 dB
 */

static int es_reader0(int argc, FAR char *argv[])
{
  while(1){
      err_t err = theAudio->writeFrames(AudioClass::Player0,mainFile);

        /*  Tell when one of player file ends */
      if (err == AUDIOLIB_ECODE_FILEEND)
        {
         printf("Main Player File End!\n");
        }

      if (err)
        {
         printf("Main player error code: %d\n", err);
         goto stop_player;
       }
    usleep(40000);
  }

stop_player:
  sleep(1);
  theAudio->stopPlayer(AudioClass::Player0);
  mainFile.close();
  exit(1);  
}

static int es_reader1(int argc, FAR char *argv[])
{
  while(1){
      err_t err = theAudio->writeFrames(AudioClass::Player1,subFile);

        /*  Tell when one of player file ends */
      if (err == AUDIOLIB_ECODE_FILEEND)
        {
         printf("Sub Player File End!\n");
        }

      if (err)
        {
         printf("Sub player error code: %d\n", err);
         goto stop_player;
       }
    usleep(40000);
  }

stop_player:
  sleep(1);
  theAudio->stopPlayer(AudioClass::Player1);
  subFile.close();
  exit(1);  
}

void setup()
{

  /* start audio system */
  theAudio = AudioClass::getInstance();

  puts("initialization Audio Library");
  theAudio->begin();

  /* Set output device to speaker */
  theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP);

  /*
   * Set main player to decode stereo mp3. Stream sample rate is set to "auto detect"
   * Search for MP3 decoder in "/mnt/sd0/BIN" directory
   */
  err_t err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_AUTO, AS_CHANNEL_STEREO);

  /* Verify player initialize */
  if (err != AUDIOLIB_ECODE_OK)
  {
    printf("Player0 initialize error\n");
    exit(1);
  }

  /*
   * Set main player to decode stereo mp3. Stream sample rate is set to "auto detect"
   * Search for MP3 decoder in "/mnt/sd0/BIN" directory
   */
  err = theAudio->initPlayer(AudioClass::Player1, AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_AUTO, AS_CHANNEL_STEREO);

  /* Verify player initialize */
  if (err != AUDIOLIB_ECODE_OK)
  {
    printf("Player1 initialize error\n");
    exit(1);
  }
  
  
  /* Open file placed on SD card */
  mainFile = theSD.open("Sound0.mp3");

  /* Verify file open */
  if (!mainFile)
    {
      printf("Main file open error\n");
      exit(1);
    }

  /* Open file placed on SD card */
  subFile =  theSD.open("Sound1.mp3");

  /* Verify file open */
  if (!subFile)
    {
      printf("Sub file open error\n");
      exit(1);
    }

  /* Send first frames to be decoded */
  err = theAudio->writeFrames(AudioClass::Player0,mainFile);
  if (err)
    {
      printf("Main player: File Read Error! =%d\n",err);
      mainFile.close();
      exit(1);
    }

  err = theAudio->writeFrames(AudioClass::Player1,subFile);
  if (err)
    {
      printf("Sub player: File Read Error! =%d\n",err);
      subFile.close();
      exit(1);
    }

  puts("Play!");

  /* Set master volume to -8.0 dB, Player0 volume to -8.0 dB, Player1 volume to -8.0 dB */
  theAudio->setVolume(-80,-80,-80);

  theAudio->startPlayer(AudioClass::Player0);
  theAudio->startPlayer(AudioClass::Player1);

  puts("start!");
  
  /* You must set higher priority than main loop(priority is 100). */
  task_create("es_reader0", 155, 1024, es_reader0, NULL);  
  task_create("es_reader1", 155, 1024, es_reader1, NULL);

}


/**
 *  @brief Play streams
 *
 * Send new frames to decode in a loop until file ends
 */
void loop()
{
  /* Do nothing on main task. */
  sleep(1);
}
