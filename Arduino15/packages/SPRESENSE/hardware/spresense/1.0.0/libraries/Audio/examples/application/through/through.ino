/*
 *  through.ino - Through mode(I2S <=> Analog) example application
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

/**
 * @brief Audio attention callback
 *
 * When audio internal error occurc, this function will be called back.
 */

static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");
  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      exit(1);
   }
}

/**
 * @brief Setup audio player to play mp3 file
 *
 * Set clock mode to normal <br>
 * Set output device to speaker <br>
 * Set main player to decode stereo mp3. Stream sample rate is set to "auto detect" <br>
 * System directory "/mnt/sd0/BIN" will be searched for MP3 decoder (MP3DEC file)
 * Open "Sound.mp3" file <br>
 * Set master volume to -16.0 dB
 */
void setup()
{

  // start audio system
  theAudio = AudioClass::getInstance();
  theAudio->begin(audio_attention_cb);

  puts("initialization Audio Library");

  /* Set output device to speaker with first argument.
   * If you want to change the output device to I2S,
   * specify "AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT" as an argument.
   * Set speaker driver mode to LineOut with second argument.
   * If you want to change the speaker driver mode to other,
   * specify "AS_SP_DRV_MODE_1DRIVER" or "AS_SP_DRV_MODE_2DRIVER" or "AS_SP_DRV_MODE_4DRIVER"
   * as an argument.
   */
//  int err = theAudio->setThroughMode(AudioClass::BothIn,AudioClass::Mic,true,160,AS_SP_DRV_MODE_LINEOUT);
  int err = theAudio->setThroughMode(AudioClass::I2sIn,AudioClass::Mixer,true,160,AS_SP_DRV_MODE_LINEOUT);
  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("Through initialize error\n");
      exit(1);
    }

  theAudio->setVolume(-160);
  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("Set Volume error\n");
      exit(1);
    }
}

/**
 */
void loop()
{
  /* 
   *  Do nothing.
  */

}
