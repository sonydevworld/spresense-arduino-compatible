/*
 *  player_wav.ino - Simple sound player example application
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

File myFile;

const int32_t sc_buffer_size = 6144;
uint8_t s_buffer[sc_buffer_size];

uint32_t s_remain_size = 0;
bool ErrEnd = false;

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
      ErrEnd = true;
    }
}

/**
 * @brief Setup audio player to play wav file
 *
 * Set clock mode to normal <br>
 * Set output device to speaker <br>
 * Set main player to decode stereo wav. Stream sample rate is auto detect. <br>
 * System directory "/mnt/sd0/BIN" will be searched for WAV decoder (WAVDEC file)
 * Open "Sound.wav" file <br>
 * Set master volume to -16.0 dB
 */

static const uint32_t sc_prestore_frames = 10;
 
void setup()
{
  theSD.begin();

  // Get wav file info

  WavContainerFormatParser *p_parser = new WavContainerFormatParser();
  fmt_chunk_t fmt;

  handel_wav_parser_t *handle = (handel_wav_parser_t *)p_parser->parseChunk("/mnt/sd0/Sound.wav", &fmt);
  if (handle == NULL)
    {
      printf("Wav parser error.\n");
      exit(1);
    }

  s_remain_size = handle->data_size + sizeof(WAVHEADER);

  p_parser->resetParser((handel_wav_parser *)handle);

  // start audio system
  theAudio = AudioClass::getInstance();

  theAudio->begin(audio_attention_cb);

  puts("initialization Audio Library");

  /* Set clock mode to normal */

  theAudio->setRenderingClockMode((fmt.rate <= 48000) ? AS_CLKMODE_NORMAL : AS_CLKMODE_HIRES);

  /* Set output device to speaker with first argument.
   * If you want to change the output device to I2S,
   * specify "AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT" as an argument.
   * Set speaker driver mode to LineOut with second argument.
   * If you want to change the speaker driver mode to other,
   * specify "AS_SP_DRV_MODE_1DRIVER" or "AS_SP_DRV_MODE_2DRIVER" or "AS_SP_DRV_MODE_4DRIVER"
   * as an argument.
   */
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT);

  /*
   * Set main player to decode wav. Initialize parameters are taken from wav header.
   * Search for WAV decoder in "/mnt/sd0/BIN" directory
   */
  err_t err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_WAV, "/mnt/sd0/BIN", fmt.rate, fmt.bit, fmt.channel);

  /* Verify player initialize */
  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("Player0 initialize error\n");
      exit(1);
    }

  /* Open file placed on SD card */
  myFile = theSD.open("Sound.wav");

  /* Verify file open */
  if (!myFile)
    {
      printf("File open error\n");
      exit(1);
    }
  printf("Open! %d\n",myFile);

  for (uint32_t i = 0; i < sc_prestore_frames; i++)
    {
      size_t supply_size = myFile.read(s_buffer, sizeof(s_buffer));
      s_remain_size -= supply_size;
      
      err = theAudio->writeFrames(AudioClass::Player0, s_buffer, supply_size);
      if (err != AUDIOLIB_ECODE_OK)
        {
          break;
        }
        
      if (s_remain_size == 0)
        {
          break;
        }
    }
    
  /* Main volume set to -16.0 dB */
    
  theAudio->setVolume(-160);
  
  theAudio->startPlayer(AudioClass::Player0);
  puts("Play!");
  }

/**
 * @brief Play stream
 *
 * Send new frames to decode in a loop until file ends
 */

static const uint32_t sc_store_frames = 10;

void loop()
{
  static bool is_carry_over = false;
  static size_t supply_size = 0;

  /* Send new frames to decode in a loop until file ends */
  for (uint32_t i = 0; i < sc_store_frames; i++)
    {
      if (!is_carry_over)
        {
          supply_size = myFile.read(s_buffer, (s_remain_size < sizeof(s_buffer)) ? s_remain_size : sizeof(s_buffer));
          s_remain_size -= supply_size;
        }
      is_carry_over = false;

      int err = theAudio->writeFrames(AudioClass::Player0, s_buffer, supply_size);

      if (err == AUDIOLIB_ECODE_SIMPLEFIFO_ERROR)
        {
          is_carry_over = true;
          break;
        }

      if (s_remain_size == 0)
        {
          goto stop_player;
        }
    }

  if (ErrEnd)
    {
      printf("Error End\n");
      goto stop_player;
    }

  /* This sleep is adjusted by the time to read the audio stream file.
     Please adjust in according with the processing contents
     being processed at the same time by Application.
  */

  usleep(1000);

  /* Don't go further and continue play */
  
  return;

stop_player:
  puts("End.");
  theAudio->stopPlayer(AudioClass::Player0);
  myFile.close();
  exit(1);
}
