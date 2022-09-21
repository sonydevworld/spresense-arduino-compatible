/*
 *  rec_play.ino - Record and playback audio example application
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

#define RecordLoopNum 800

enum
{
  PlayReady = 0,
  Playing,
  RecordReady,
  Recording
};

SDClass theSD;
AudioClass *theAudio;

File myFile;

bool ErrEnd = false;

/**
 * @brief Audio attention callback
 *
 * When audio internal error occurs, this function will be called back.
 */

static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");
  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      ErrEnd = true;
   }
}

/**
 * @brief Set up audio library to record and play
 *
 * Get instance of audio library and begin.
 * Set rendering clock to NORMAL(48kHz).
 */
void setup()
{
  /* Initialize SD */
  while (!theSD.begin())
    {
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
    }

  // start audio system
  theAudio = AudioClass::getInstance();

  theAudio->begin(audio_attention_cb);

  puts("initialization Audio Library");

  /* Set output device to speaker */
  theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);
}

/**
 * @brief Audio player set up and start
 *
 * Set audio player to play mp3/Stereo audio.
 * Audio data is read from SD card.
 *
 */
void playerMode(char *fname)
{
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

  /* Open file placed on SD card */
  myFile = theSD.open(fname);

  /* Verify file open */
  if (!myFile)
    {
      printf("File open error\n");
      exit(1);
    }
  printf("Open! 0x%08lx\n", (uint32_t)myFile);

  /* Send first frames to be decoded */
  err = theAudio->writeFrames(AudioClass::Player0, myFile);

  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("File Read Error! =%d\n",err);
      myFile.close();
      exit(1);
    }

  puts("Play!");

  /* Main volume set to -16.0 dB, Main player and sub player set to 0 dB */
  theAudio->setVolume(-160, 0, 0);
  theAudio->startPlayer(AudioClass::Player0);
}

/**
 * @brief Supply audio data to the buffer
 */
bool playStream()
{
  /* Send new frames to decode in a loop until file ends */
  err_t err = theAudio->writeFrames(AudioClass::Player0, myFile);

  /*  Tell when player file ends */
  if (err == AUDIOLIB_ECODE_FILEEND)
    {
      printf("Main player File End!\n");
    }

  /* Show error code from player and stop */
  if (err)
    {
      printf("Main player error code: %d\n", err);
      goto stop_player;
    }

  /* This sleep is adjusted by the time to read the audio stream file.
   * Please adjust in according with the processing contents
   * being processed at the same time by Application.
   *
   * The usleep() function suspends execution of the calling thread for usec
   * microseconds. But the timer resolution depends on the OS system tick time
   * which is 10 milliseconds (10,000 microseconds) by default. Therefore,
   * it will sleep for a longer time than the time requested here.
   */

  usleep(40000);
  
  /* Don't go further and continue play */
  return false;

stop_player:
  sleep(1);
  theAudio->stopPlayer(AudioClass::Player0);
  myFile.close();

  /* return play end */
  return true;
}

/**
 * @brief Audio recorder set up and start
 *
 * Set audio recorder to record mp3/Stereo audio.
 * Audio data is written to SD card.
 *
 */
void recorderMode(char *fname)
{
  /* Select input device as analog microphone */
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);

  /*
   * Initialize filetype to stereo mp3 with 48 Kb/s sampling rate
   * Search for MP3 codec in "/mnt/sd0/BIN" directory
   */
  theAudio->initRecorder(AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_48000, AS_CHANNEL_STEREO);

  /* Open file for data write on SD card */
  myFile = theSD.open(fname, FILE_WRITE);
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
 * @brief Read audio data from the buffer
 */
bool recordStream()
{
  static int cnt = 0;
  err_t err = AUDIOLIB_ECODE_OK;

  /* recording end condition */
  if (cnt > RecordLoopNum)
    {
      puts("End Recording");
      goto stop_recorder;
    }

  /* Read frames to record in file */
  err = theAudio->readFrames(myFile);

  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("File End! =%d\n",err);
      goto stop_recorder;
    }

  /* This sleep is adjusted by the time to write the audio stream file.
   * Please adjust in according with the processing contents
   * being processed at the same time by Application.
   *
   * The usleep() function suspends execution of the calling thread for usec
   * microseconds. But the timer resolution depends on the OS system tick time
   * which is 10 milliseconds (10,000 microseconds) by default. Therefore,
   * it will sleep for a longer time than the time requested here.
   */

  usleep(10000);
  cnt++;

  /* return still recording */
  return false;

stop_recorder:
  sleep(1);
  theAudio->stopRecorder();
  theAudio->closeOutputFile(myFile);
  myFile.close();
  cnt = 0;

  /* return record end */
  return true;
}

/**
 * @brief Rec and play streams
 *
 * Repeat recording and playing stream files
 */
void loop()
{
  static int s_state = RecordReady;
  static int fcnt = 0;
  char fname[16] = "RecPlay";
  puts("loop!!");

  /* Rec and play 5 times */
  if (fcnt >= 5)
    {
      puts("End application");
      exit(1);
    }

  if (ErrEnd)
    {
      printf("Error End\n");
      exit(1);
    }

  switch (s_state)
    {
      case PlayReady:
        snprintf(fname, sizeof(fname), "RecPlay%d.mp3", fcnt);
        playerMode(fname);
        s_state = Playing;
        break;
        
      case Playing:
        if (playStream())
          {
            theAudio->setReadyMode();
            s_state = RecordReady;
            fcnt++;
          }
        break;
        
      case RecordReady:
        snprintf(fname, sizeof(fname), "RecPlay%d.mp3", fcnt);
        recorderMode(fname);
        s_state = Recording;
        break;
        
      case Recording:
        if (recordStream())
          {
            theAudio->setReadyMode();
            s_state = PlayReady;
          }
        break;

      default:
        break;
    }
}
