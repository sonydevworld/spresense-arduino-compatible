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
#include <EEPROM.h>
#include <audio/utilities/playlist.h>
#include <stdio.h>
#include <stdlib.h>

SDClass theSD;
AudioClass *theAudio;
Playlist thePlaylist("TRACK_DB.CSV");
Track currentTrack;

int eeprom_idx = 0;
struct SavedObject {
  int saved;
  int volume;
  int random;
  int repeat;
  int autoplay;
} preset;

File myFile;

bool ErrEnd = false;

static void menu()
{
  printf("=== MENU (input key ?) ==============\n");
  printf("p: play  s: stop  +/-: volume up/down\n");
  printf("l: list  n: next  b: back\n");
  printf("r: repeat on/off  R: random on/off\n");
  printf("a: auto play      m,h,?: menu\n");
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

static bool next()
{
  if (thePlaylist.getNextTrack(&currentTrack)) {
    show(&currentTrack);
    return true;
  } else {
    return false;
  }
}

static bool prev()
{
  if (thePlaylist.getPrevTrack(&currentTrack)) {
    show(&currentTrack);
    return true;
  } else {
    return false;
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

  /* Prepare for playback with the specified track */

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
  const char *playlist_dirname = "/mnt/sd0/PLAYLIST";
  bool success;

  /* Display menu */

  Serial.begin(115200);
  menu();

  /* Load preset data */

  EEPROM.get(eeprom_idx, preset);
  if (!preset.saved) {
    /* If no preset data, come here */
    preset.saved = 1;
    preset.volume = -160; /* default */
    preset.random = 0;
    preset.repeat = 0;
    preset.autoplay = 0;
    EEPROM.put(eeprom_idx, preset);
  }
  printf("Volume=%d\n", preset.volume);
  printf("Random=%s\n", (preset.random) ? "On" : "Off");
  printf("Repeat=%s\n", (preset.repeat) ? "On" : "Off");
  printf("Auto=%s\n", (preset.autoplay) ? "On" : "Off");

  /* Initialize SD */
  while (!theSD.begin())
    {
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
    }

  /* Initialize playlist */

  success = thePlaylist.init(playlist_dirname);
  if (!success) {
    printf("ERROR: no exist playlist file %s/TRACK_DB.CSV\n",
           playlist_dirname);
    while (1);
  }

  /* Set random seed to use shuffle mode */

  struct timespec ts;
  clock_systime_timespec(&ts);
  srand((unsigned int)ts.tv_nsec);

  /* Restore preset data */

  if (preset.random) {
    thePlaylist.setPlayMode(Playlist::PlayModeShuffle);
  }

  if (preset.repeat) {
    thePlaylist.setRepeatMode(Playlist::RepeatModeOn);
  }

  thePlaylist.getNextTrack(&currentTrack);

  if (preset.autoplay) {
    show(&currentTrack);
  }

  /* Start audio system */

  theAudio = AudioClass::getInstance();

  theAudio->begin(audio_attention_cb);

  /* Set main volume */

  theAudio->setVolume(preset.volume);

}

void loop()
{
  static enum State {
    Stopped,
    Ready,
    Active
  } s_state = preset.autoplay ? Ready : Stopped;
  err_t err = AUDIOLIB_ECODE_OK;

  /* Fatal error */
  if (ErrEnd) {
    printf("Error End\n");
    goto stop_player;
  }

  /* Menu operation */

  if (Serial.available() > 0) {
    switch (Serial.read()) {
    case 'a': // autoplay
      if (preset.autoplay) {
        preset.autoplay = 0;
      } else {
        preset.autoplay = 1;
      }
      printf("Auto=%s\n", (preset.autoplay) ? "On" : "Off");
      EEPROM.put(eeprom_idx, preset);
      break;
    case 'p': // play
      if (s_state == Stopped) {
        s_state = Ready;
        show(&currentTrack);
      }
      break;
    case 's': // stop
      if (s_state == Active) {
        stop();
      }
      s_state = Stopped;
      break;
    case '+': // volume up
      preset.volume += 10;
      if (preset.volume > 120) {
        /* set max volume */
        preset.volume = 120;
      }
      printf("Volume=%d\n", preset.volume);
      theAudio->setVolume(preset.volume);
      EEPROM.put(eeprom_idx, preset);
      break;
    case '-': // volume down
      preset.volume -= 10;
      if (preset.volume < -1020) {
        /* set min volume */
        preset.volume = -1020;
      }
      printf("Volume=%d\n", preset.volume);
      theAudio->setVolume(preset.volume);
      EEPROM.put(eeprom_idx, preset);
      break;
    case 'l': // list
      if (preset.repeat) {
        thePlaylist.setRepeatMode(Playlist::RepeatModeOff);
        list();
        thePlaylist.setRepeatMode(Playlist::RepeatModeOn);
      } else {
        list();
      }
      break;
    case 'n': // next
      if (s_state == Ready) {
        // do nothing
      } else { // s_state == Active or Stopped
        if (s_state == Active) {
          stop();
          s_state = Ready;
        }
        if (!next()) {
          s_state = Stopped;
        }
      }
      break;
    case 'b': // back
      if (s_state == Active) {
        stop();
        s_state = Ready;
      }
      prev();
      break;
    case 'r': // repeat
      if (preset.repeat) {
        preset.repeat = 0;
        thePlaylist.setRepeatMode(Playlist::RepeatModeOff);
      } else {
        preset.repeat = 1;
        thePlaylist.setRepeatMode(Playlist::RepeatModeOn);
      }
      printf("Repeat=%s\n", (preset.repeat) ? "On" : "Off");
      EEPROM.put(eeprom_idx, preset);
      break;
    case 'R': // random
      if (preset.random) {
        preset.random = 0;
        thePlaylist.setPlayMode(Playlist::PlayModeNormal);
      } else {
        preset.random = 1;
        thePlaylist.setPlayMode(Playlist::PlayModeShuffle);
      }
      printf("Random=%s\n", (preset.random) ? "On" : "Off");
      EEPROM.put(eeprom_idx, preset);
      break;
    case 'm':
    case 'h':
    case '?':
      menu();
      break;
    default:
      break;
    }
  }

  /* Processing in accordance with the state */

  switch (s_state) {
  case Stopped:
    break;

  case Ready:
    if (start()) {
      s_state = Active;
    } else {
      goto stop_player;
    }
    break;

  case Active:
    /* Send new frames to be decoded until end of file */

    err = theAudio->writeFrames(AudioClass::Player0, myFile);

    if (err == AUDIOLIB_ECODE_FILEEND) {
      /* Stop player after playback until end of file */

      theAudio->stopPlayer(AudioClass::Player0, AS_STOPPLAYER_ESEND);
      myFile.close();
      if (next()) {
        s_state = Ready;
      } else {
        /* If no next track, stop the player */
        s_state = Stopped;
      }
    } else if (err != AUDIOLIB_ECODE_OK) {
      printf("Main player error code: %d\n", err);
      goto stop_player;
    }
    break;

  default:
    break;
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

  usleep(1000);

  return;

stop_player:
  theAudio->stopPlayer(AudioClass::Player0);
  myFile.close();
  theAudio->setReadyMode();
  theAudio->end();
  printf("Exit player\n");
  exit(1);
}
