/*
 *  player_playlist.ino - Sound player example application by playlist
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

#include <SDHCI.h>
#include <Audio.h>
#include <audio/utilities/playlist.h>
#include <stdio.h>

SDClass theSD;
AudioClass *theAudio;
Playlist thePlaylist("TRACK_DB.CSV");
Track currentTrack;
int volume = -160;

File myFile;

bool ErrEnd = false;

static void menu()
{
  printf("=== MENU (input key ?) ==============\n");
  printf("p: play  s: stop  +/-: volume up/down\n");
  printf("l: list  n: next  b: back\n");
  printf("r: repeat on/off  R: random on/off\n");
  printf("=====================================\n");
}

static void show(Track *t)
{
  printf("%s | %s | %s\n", t->author, t->album, t->title);
}

static void list()
{
  Track t;
  thePlaylist.restart();
  printf("-----------------------------\n");
  while (thePlaylist.getNextTrack(&t)) {
    if (0 == strncmp(currentTrack.title, t.title, 64)) {
      printf("-> ");
    }
    show(&t);
  }
  printf("-----------------------------\n");

  /* restore the current track */
  thePlaylist.restart();
  while (thePlaylist.getNextTrack(&t)) {
    if (0 == strncmp(currentTrack.title, t.title, 64)) {
      break;
    }
  }
}

static void next()
{
  if (thePlaylist.getNextTrack(&currentTrack)) {
    show(&currentTrack);
  }
}

static void prev()
{
  if (thePlaylist.getPrevTrack(&currentTrack)) {
    show(&currentTrack);
  }
}

static err_t setPlayer(Track *t)
{
  static uint8_t   s_codec   = 0;
  static uint32_t  s_fs      = 0;
  static uint8_t   s_bitlen  = 0;
  static uint8_t   s_channel = 0;
  static AsClkMode s_clkmode = (AsClkMode)-1;
  err_t err = AUDIOLIB_ECODE_OK;
  AsClkMode clkmode;

  if ((s_codec   != t->codec_type) ||
      (s_fs      != t->sampling_rate) ||
      (s_bitlen  != t->bit_length) ||
      (s_channel != t->channel_number)) {

    /* Set audio clock 48kHz/192kHz */

    clkmode = (t->sampling_rate <= 48000) ? AS_CLKMODE_NORMAL : AS_CLKMODE_HIRES;

    if (s_clkmode != clkmode) {

      /* When the audio master clock will be changed, it should change the clock
       * mode once after returning the ready state. At the first start-up, it
       * doesn't need to call setReadyMode().
       */

      if (s_clkmode != (AsClkMode)-1) {
        theAudio->setReadyMode();
      }

      s_clkmode = clkmode;

      theAudio->setRenderingClockMode(clkmode);

      /* Set output device to speaker with first argument.
       * If you want to change the output device to I2S,
       * specify "AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT" as an argument.
       * Set speaker driver mode to LineOut with second argument.
       * If you want to change the speaker driver mode to other,
       * specify "AS_SP_DRV_MODE_1DRIVER" or "AS_SP_DRV_MODE_2DRIVER" or "AS_SP_DRV_MODE_4DRIVER"
       * as an argument.
       */

      theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT);
    }

    /* Initialize player */

    err = theAudio->initPlayer(AudioClass::Player0,
                               t->codec_type,
                               "/mnt/sd0/BIN",
                               t->sampling_rate,
                               t->bit_length,
                               t->channel_number);
    if (err != AUDIOLIB_ECODE_OK) {
      printf("Player0 initialize error\n");
      return err;
    }

    s_codec   = t->codec_type;
    s_fs      = t->sampling_rate;
    s_bitlen  = t->bit_length;
    s_channel = t->channel_number;
  }

  return err;
}

static bool start()
{
  err_t err;
  Track *t = &currentTrack;

  /* Set Player */

  err = setPlayer(t);

  if (err != AUDIOLIB_ECODE_OK) {
    return false;
  }

  /* Open file placed on SD card */

  char fullpath[64] = { 0 };
  snprintf(fullpath, sizeof(fullpath), "AUDIO/%s", t->title);

  myFile = theSD.open(fullpath);

  if (!myFile){
    printf("File open error\n");
    return false;
  }

  /* Send first frames to be decoded */

  err = theAudio->writeFrames(AudioClass::Player0, myFile);

  if ((err != AUDIOLIB_ECODE_OK) && (err != AUDIOLIB_ECODE_FILEEND)) {
    printf("File Read Error! =%d\n",err);
    return false;
  }

  printf("start\n");
  theAudio->startPlayer(AudioClass::Player0);

  return true;
}

static void stop()
{
  printf("stop\n");
  theAudio->stopPlayer(AudioClass::Player0, AS_STOPPLAYER_NORMAL);
  myFile.close();
}

static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");

  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING) {
    ErrEnd = true;
  }
}

void setup()
{
  Serial.begin(115200);

  theSD.begin();

  /* init playlist */

  thePlaylist.init("/mnt/sd0/PLAYLIST");
  thePlaylist.getNextTrack(&currentTrack);

  /* start audio system */

  theAudio = AudioClass::getInstance();

  theAudio->begin(audio_attention_cb);

  /* Main volume set to default */

  theAudio->setVolume(volume);

  /* Menu display */

  menu();

}

void loop()
{
  static enum State {
    Stopped,
    Ready,
    Active
  } s_state = Stopped;
  static bool s_repeat = false;
  static bool s_random = false;
  err_t err = AUDIOLIB_ECODE_OK;

  /* Fatal error */
  if (ErrEnd) {
    printf("Error End\n");
    goto stop_player;
  }

  /* Menu operation */
  if (Serial.available() > 0) {
    switch (Serial.read()) {
    case 'p': // play
      if (s_state == Stopped) {
        s_state = Ready;
        show(&currentTrack);
      }
      break;
    case 's': // stop
      stop();
      s_state = Stopped;
      break;
    case '+': // volume up
      volume += 10;
      if (volume > 120)
        volume = 120;
      printf("Volume=%d\n", volume);
      theAudio->setVolume(volume);
      break;
    case '-': // volume down
      volume -= 10;
      if (volume < -1020)
        volume = -1020;
      printf("Volume=%d\n", volume);
      theAudio->setVolume(volume);
      break;
    case 'l': // list
      if (s_repeat) {
        thePlaylist.setRepeatMode(Playlist::RepeatModeOff);
        list();
        thePlaylist.setRepeatMode(Playlist::RepeatModeOn);
      } else {
        list();
      }
      break;
    case 'n': // next
      if (s_state != Stopped) {
        stop();
        s_state = Ready;
      }
      next();
      break;
    case 'b': // back
      if (s_state != Stopped) {
        stop();
        s_state = Ready;
      }
      prev();
      break;
    case 'r': // repeat
      if (s_repeat) {
        s_repeat = false;
        thePlaylist.setRepeatMode(Playlist::RepeatModeOff);
      } else {
        s_repeat = true;
        thePlaylist.setRepeatMode(Playlist::RepeatModeOn);
      }
      printf("Repeat=%s\n", (s_repeat) ? "On" : "Off");
      break;
    case 'R': // random
      if (s_random) {
        s_random = false;
        thePlaylist.setPlayMode(Playlist::PlayModeNormal);
      } else {
        s_random = true;
        thePlaylist.setPlayMode(Playlist::PlayModeShuffle);
        if (s_state == Stopped) {
          // When random mode is entered in stopped state,
          // skip to prevent the same title from being selected.
          next();
          next();
        }
      }
      printf("Random=%s\n", (s_random) ? "On" : "Off");
      break;
    case 'm':
    case 'h':
      menu();
      break;
    default:
      break;
    }
  }

  switch (s_state) {
  case Stopped:
    break;

  case Ready:
    if (!start()) {
      goto stop_player;
    } else {
      s_state = Active;
    }
    break;

  case Active:
    /* Send new frames to decode in a loop until file ends */

    err = theAudio->writeFrames(AudioClass::Player0, myFile);

    /*  Tell when player file ends */

    if (err == AUDIOLIB_ECODE_FILEEND) {
      stop();
      next();
      s_state = Ready;
    } else if (err != AUDIOLIB_ECODE_OK) {
      printf("Main player error code: %d\n", err);
      goto stop_player;
    }
    break;

  default:
    break;
  }

  /* This sleep is adjusted by the time to read the audio stream file.
     Please adjust in according with the processing contents
     being processed at the same time by Application.
   */

  usleep(1000);

  /* Don't go further and continue play */

  return;

stop_player:
  printf("Exit player\n");
  myFile.close();
  exit(1);
}
