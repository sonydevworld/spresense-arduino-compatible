/*
 *  recorder_wav_objIf.ino - Object I/F based sound recorder example application
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

#include <SDHCI.h>
#include <MediaRecorder.h>
#include <MemoryUtil.h>

#define RECORD_FILE_NAME "Sound.wav"

MediaRecorder *theRecorder;
SDClass theSD;

File s_myFile;

bool ErrEnd = false;

/**
 * @brief Audio attention callback
 *
 * When audio internal error occurs, this function will be called back.
 */

static void mediarecorder_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");
  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      ErrEnd = true;
   }
}


/* Sampling rate
 * Set 16000 or 48000
 */

static const uint32_t recoding_sampling_rate = 48000;

/* Number of input channels
 * Set either 1, 2, or 4.
 */

static const uint8_t  recoding_cannel_number = 2;

/* Audio bit depth
 * Set 16 or 24
 */

static const uint8_t  recoding_bit_length = 16;

/* Recording time[second] */

static const uint32_t recoding_time = 10;

/* Bytes per second */

static const int32_t recoding_byte_per_second = recoding_sampling_rate *
                                                recoding_cannel_number *
                                                recoding_bit_length / 8;

/* Total recording size */

static const int32_t recoding_size = recoding_byte_per_second * recoding_time;

/* One frame size
 * Calculated with 768 samples per frame.
 */

static const uint32_t frame_size  = 768 * recoding_cannel_number * (recoding_bit_length / 8);

/* Buffer size
 * Align in 512byte units based on frame size.
 */

static const uint32_t buffer_size = (frame_size + 511) & ~511;
static uint8_t        s_buffer[buffer_size];

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
  printf("mp cb %x %lx %lx\n", event, result, sub_result);

  return true;
}

/**
 * @brief Setup Recorder
 *
 * Set input device to Mic <br>
 * Initialize recorder to encode stereo wav stream with 48kHz sample rate <br>
 * System directory "/mnt/sd0/BIN" will be searched for SRC filter (SRC file)
 * Open RECORD_FILE_NAME file <br>
 */

void setup()
{
  /* Initialize memory pools and message libs */

  initMemoryPools();
  createStaticPools(MEM_LAYOUT_RECORDER);

  /* start audio system */

  theRecorder = MediaRecorder::getInstance();

  theRecorder->begin(mediarecorder_attention_cb);

  puts("initialization MediaRecorder");

  /* Set capture clock */

  theRecorder->setCapturingClkMode(MEDIARECORDER_CAPCLK_NORMAL);

  /* Activate Objects. Set output device to Speakers/Headphones */

  theRecorder->activate(AS_SETRECDR_STS_INPUTDEVICE_MIC, mediarecorder_done_callback);

  usleep(100 * 1000); /* waiting for Mic startup */

  /* Initialize SD */
  while (!theSD.begin())
    {
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
    }

  /*
   * Initialize recorder to decode stereo wav stream with 48kHz sample rate
   * Search for SRC filter in "/mnt/sd0/BIN" directory
   */

  theRecorder->init(AS_CODECTYPE_WAV,
                    recoding_cannel_number,
                    recoding_sampling_rate,
                    recoding_bit_length,
                    AS_BITRATE_8000, /* Bitrate is effective only when mp3 recording */
                    "/mnt/sd0/BIN");

  /* Open file for data write on SD card */

  if (theSD.exists(RECORD_FILE_NAME))
    {
      printf("Remove existing file [%s].\n", RECORD_FILE_NAME);
      theSD.remove(RECORD_FILE_NAME);
    }

  s_myFile = theSD.open(RECORD_FILE_NAME, FILE_WRITE);

  /* Verify file open */

  if (!s_myFile)
    {
      printf("File open error\n");
      exit(1);
    }

  printf("Open! [%s]\n", RECORD_FILE_NAME);

  /* Write wav header (Write to top of file. File size is tentative.) */

  theRecorder->writeWavHeader(s_myFile);
  puts("Write Header!");
  
  /* Start Recorder */

  theRecorder->start();
  puts("Recording Start!");

}
/**
 * @brief Audio signal process (Modify for your application)
 */
void signal_process(uint32_t size)
{
  /* Put any signal process */

  printf("Size %ld [%02x %02x %02x %02x %02x %02x %02x %02x ...]\n",
         size,
         s_buffer[0],
         s_buffer[1],
         s_buffer[2],
         s_buffer[3],
         s_buffer[4],
         s_buffer[5],
         s_buffer[6],
         s_buffer[7]);
}

/**
 * @brief Execute one frame
 */
err_t execute_aframe(uint32_t* size)
{
  err_t err = theRecorder->readFrames(s_buffer, buffer_size, size);

  if(((err == MEDIARECORDER_ECODE_OK) || (err == MEDIARECORDER_ECODE_INSUFFICIENT_BUFFER_AREA)) && (*size > 0))
    {
      signal_process(*size);
    }else{
      return err;
    }
  int ret = s_myFile.write((uint8_t*)&s_buffer, *size);
  if (ret < 0)
    {
      puts("File write error.");
      err = MEDIARECORDER_ECODE_FILEACCESS_ERROR;
    }

  return err;
}

/**
 * @brief Execute frames for FIFO empty
 */
void execute_frames()
{
  uint32_t read_size = 0;
  do
    {
      err_t err = execute_aframe(&read_size);
      if ((err != MEDIARECORDER_ECODE_OK)
       && (err != MEDIARECORDER_ECODE_INSUFFICIENT_BUFFER_AREA))
        {
          break;
        }
    }
  while (read_size > 0);
}

/**
 * @brief Record audio frames
 */

void loop()
{
    static int32_t total_size = 0;
    uint32_t read_size = 0;

  /* Execute audio data */
  err_t err = execute_aframe(&read_size);
  if (err != MEDIARECORDER_ECODE_OK && err != MEDIARECORDER_ECODE_INSUFFICIENT_BUFFER_AREA)
    {
      puts("Recording Error!");
      theRecorder->stop();
      goto exitRecording;
    }
  else if (read_size>0)
    {
      total_size += read_size;
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

//  usleep(10000);

  /* Stop Recording */
  if (total_size > recoding_size)
    {
      theRecorder->stop();

      /* Get ramaining data(flushing) */
      sleep(1); /* For data pipline stop */
      execute_frames();
      
      goto exitRecording;
    }

  if (ErrEnd)
    {
      printf("Error End\n");
      theRecorder->stop();
      goto exitRecording;
    }

  return;

exitRecording:

  theRecorder->writeWavHeader(s_myFile);
  puts("Update Header!");

  s_myFile.close();

  theRecorder->deactivate();
  theRecorder->end();
  
  puts("End Recording");
  exit(1);

}
