/*
 *  recorder_objIf.ino - Object I/F based sound recorder example application
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
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,  MA 02110-1301  USA
 */

#include <MediaRecorder.h>
#include <MemoryUtil.h>

#define RECORD_LOOP 400

MediaRecorder *theRecorder;
SDClass theSD;
File s_myFile;

static const int32_t sc_buffer_size = 6144;
static uint8_t s_buffer[sc_buffer_size];

/**
 * @brief Recorder done callback procedure
 *
 * @param [in] event        AsRecorderEvent type indicator
 * @param [in] result       Result
 * @param [in] sub_result   Sub result
 *
 * @return true on success, false otherwise
 */

static bool mediarecorder_done_callback(AsRecorderEvent event, uint32_t result, uint32_t sub_result)
{
  printf("mp cb %x %x %x\n", event, result, sub_result);

  return true;
}

/**
 * @brief Setup Recorder
 *
 * Set input device to Mic <br>
 * Initialize recorder to encode stereo mp3 stream with 48kHz sample rate <br>
 * System directory "/mnt/sd0/BIN" will be searched for MP3 encoder (MP3ENC file)
 * Open "Sound.mp3" file <br>
 */

void setup()
{
  /* Initialize memory pools and message libs */

  initMemoryPools();
  createStaticPools(MEM_LAYOUT_RECORDER);

  /* start audio system */

  theRecorder = MediaRecorder::getInstance();

  theRecorder->begin();

  puts("initialization MediaRecorder");

  /* Activate Objects. Set output device to Speakers/Headphones */

  theRecorder->activate(AS_SETRECDR_STS_INPUTDEVICE_MIC, mediarecorder_done_callback);

  usleep(100 * 1000);

  /*
   * Initialize main player to decode stereo mp3 stream with 48 kb/s sample rate
   * Search for MP3 codec in "/mnt/sd0/BIN" directory
   */

  theRecorder->init(AS_CODECTYPE_MP3,
                    AS_CHANNEL_STEREO,
                    AS_SAMPLINGRATE_48000,
                    AS_BITLENGTH_16,
                    AS_BITRATE_96000,
                    "/mnt/sd0/BIN");

  s_myFile = theSD.open("Sound.mp3", FILE_WRITE);

  /* Verify file open */

  if (!s_myFile)
    {
      printf("File open error\n");
      exit(1);
    }

  printf("Open! %d\n", s_myFile);

  /* Start Recorder */

  theRecorder->start();
}

/**
 * @brief Get audio data
 */

void getEs()
{
  uint32_t read_size = 0;
  err_t err = MEDIARECORDER_ECODE_OK;

  do
    {
      err = theRecorder->readFrames(s_buffer, sc_buffer_size, &read_size);

      if ((err != MEDIARECORDER_ECODE_OK)
       && (err != MEDIARECORDER_ECODE_INSUFFICIENT_BUFFER_AREA))
        {
          break;
        }

      if (read_size > 0)
        {
          int ret = s_myFile.write((uint8_t*)&s_buffer, read_size);

          if (ret < 0)
            {
              puts("File write error.");
            }
        }
    }
  while (read_size > 0);
}

/**
 * @brief Record audio frames
 */

void loop()
{
  static int cnt = 0;

  if (cnt > RECORD_LOOP)
    {
      /* Recording end condition */

      puts("Stop Recording");
      goto exitRecording;
    }

  /* Get audio data */

  getEs();

  usleep(1);
  cnt++;

  return;

exitRecording:

  theRecorder->stop();

  /* Get ramaining data(flushing) */

  sleep(1);
  getEs();

  s_myFile.close();

  theRecorder->deactivate();
  theRecorder->end();

  exit(1);
}
