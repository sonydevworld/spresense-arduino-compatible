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

/* File name to play */

#define PLAY0_FILE_NAME   "Sound0.mp3"
#define PLAY1_FILE_NAME   "Sound1.mp3"

/* Set volume[db] */

#define VOLUME_MASTER     -160
#define VOLUME_PLAY0      -160
#define VOLUME_PLAY1      -160

SDClass theSD;

/**
 *  @brief Repeated transfer of data read into the decoder
 */

void play_process( AudioClass *theAudio, AudioClass::PlayerId play_id, File& file)
{
  while (1)
    {
      /* Send frames to be decoded */

      err_t err = theAudio->writeFrames(play_id, file);

      /*  Tell when one of player file ends */

      if (err == AUDIOLIB_ECODE_FILEEND)
        {
          printf("Player%d File End!\n", play_id);
          break;
        }
      else if (err != AUDIOLIB_ECODE_OK)
        {
           printf("Player%d error code: %d\n", play_id, err);
           break;
        }

      /* The usleep() function suspends execution of the calling thread for usec
       * microseconds. But the timer resolution depends on the OS system tick time
       * which is 10 milliseconds (10,000 microseconds) by default. Therefore,
       * it will sleep for a longer time than the time requested here.
       */

      usleep(40000);
    }
}

/**
 *  @brief Setup main audio player and sub audio player
 *
 *  Set output device to speaker <br>
 *  Open two stream files "Sound0.mp3" and "Sound1.mp3" <br>
 *  These should be in the root directory of the SD card. <br>
 *  Set main player to decode stereo mp3. Stream sample rate is set to "auto detect" <br>
 *  System directory "/mnt/sd0/BIN" will be searched for MP3 decoder (MP3DEC file) <br>
 *  This is the /BIN directory on the SD card. <br>
 */

static int player_thread(int argc, FAR char *argv[])
{
  AudioClass::PlayerId  play_id;
  err_t                 err;
  File                  file;
  const char           *file_name;

  /* Get static audio instance */

  AudioClass *theAudio = AudioClass::getInstance();

  /* Get information by task */

  play_id   = (AudioClass::PlayerId)atoi(argv[1]);
  file_name = argv[2];

  printf("\"%s\" task start\n", argv[0]);

  /* Continue playing the same file. */

  while (1)
    {
      /*
       * Set player to decode stereo mp3. Stream sample rate is set to "auto detect"
       * Search for MP3 decoder in "/mnt/sd0/BIN" directory
       */

      err = theAudio->initPlayer(play_id, AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_AUTO, AS_CHANNEL_STEREO);

      /* Verify player initialize */

      if (err != AUDIOLIB_ECODE_OK)
        {
          printf("Player%d initialize error\n", play_id);
          break;
        }

      printf("Open \"%s\" file\n", file_name);

      /* Open file placed on SD card */

      file = theSD.open(file_name);

      /* Verify file open */

      if (!file)
        {
          printf("Player%d file open error\n", play_id);
          break;
        }

      /* Send first frames to be decoded */

      err = theAudio->writeFrames(play_id, file);

      if (err != AUDIOLIB_ECODE_OK && err != AUDIOLIB_ECODE_FILEEND)
        {
          printf("Player%d: File Read Error! =%d\n", play_id, err);
          file.close();
          break;
        }

      printf("Play%d!\n", play_id);

      /* Play! */

      theAudio->startPlayer(play_id);

      printf("Start player%d!\n", play_id);

      /* Running... */

      play_process(theAudio, play_id, file);

      /* Stop! */

      theAudio->stopPlayer(play_id);

      file.close();
    }

  printf("Exit task(%d).\n", play_id);

  exit(1);

  return 0;
}

void setup()
{
  /* Get static audio instance */

  AudioClass  *theAudio = AudioClass::getInstance();

  puts("Initialization Audio Library");

  /* start audio system */

  theAudio->begin();

  /* Mount SD card */

  while (!theSD.begin())
    {
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
    }

  /* Set output device to speaker */

  theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP);

  /* Set master volume, Player0 volume, Player1 volume */

  theAudio->setVolume(VOLUME_MASTER, VOLUME_PLAY0, VOLUME_PLAY1);

  /* Initialize task parameter. */

  const char *argv0[3];
  const char *argv1[3];
  char        play_no0[4];
  char        play_no1[4];

  snprintf(play_no0, 4, "%d", AS_PLAYER_ID_0);
  snprintf(play_no1, 4, "%d", AS_PLAYER_ID_1);

  argv0[0] = play_no0;
  argv0[1] = PLAY0_FILE_NAME;
  argv0[2] = NULL;
  argv1[0] = play_no1;
  argv1[1] = PLAY1_FILE_NAME;
  argv1[2] = NULL;

  /* Start task */

  task_create("player_thread0", 155, 2048, player_thread, (char* const*)argv0);
  task_create("player_thread1", 155, 2048, player_thread, (char* const*)argv1);
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
