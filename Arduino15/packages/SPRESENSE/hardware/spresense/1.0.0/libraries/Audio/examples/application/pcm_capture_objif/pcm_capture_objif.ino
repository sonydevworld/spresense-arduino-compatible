/*
 *  pcm_capture_objif.ino - PCM capture using object if example application
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

#include <MediaRecorder.h>
#include <MemoryUtil.h>

MediaRecorder *theRecorder;

static const int32_t recoding_frames = 400;
static const int32_t buffer_size = 6144;
static uint8_t       s_buffer[buffer_size];

bool ErrEnd = false;

/**
 * @brief Audio attention callback
 *
 * When audio internal error occurc, this function will be called back.
 */

void mediarecorder_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");
  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      ErrEnd = true;
   }
}

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
 *  @brief Setup audio device to capture PCM stream
 *
 *  Select input device as microphone <br>
 *  Set PCM capture sapling rate parameters to 48 kb/s <br>
 *  Set channel number 4 to capture audio from 4 microphones simultaneously <br>
 *  System directory "/mnt/sd0/BIN" will be searched for PCM codec
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

  /*
   * Initialize recorder to decode stereo wav stream with 48kHz sample rate
   * Search for SRC filter in "/mnt/sd0/BIN" directory
   */

  theRecorder->init(AS_CODECTYPE_LPCM,
                    AS_CHANNEL_4CH,
                    AS_SAMPLINGRATE_48000,
                    AS_BITLENGTH_16,
                    AS_BITRATE_8000, /* Bitrate is effective only when mp3 recording */
                    "/mnt/sd0/BIN");

  theRecorder->start();
  puts("Recording Start!");
  
}

/**
 * @brief Audio signal process (Modify for your application)
 */

void signal_process(uint32_t size)
{
  /* Put any signal process */

  printf("Size %d [%02x %02x %02x %02x %02x %02x %02x %02x ...]\n",
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
 * @brief Execute one frame
 */

err_t execute_aframe(uint32_t* size)
{
  err_t err = theRecorder->readFrames(s_buffer, buffer_size, size);

  if(((err == MEDIARECORDER_ECODE_OK) || (err == MEDIARECORDER_ECODE_INSUFFICIENT_BUFFER_AREA)) && (*size > 0)) 
    {
      signal_process(*size);
    }

  return err;
}

/**
 * @brief Capture frames of PCM data into buffer
 */

void loop() {

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
     Please adjust in according with the processing contents
     being processed at the same time by Application.
  */
//  usleep(10000);


  /* Stop Recording */
  if (total_size > (recoding_frames*buffer_size))
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

  theRecorder->deactivate();
  theRecorder->end();
  
  puts("End Recording");
  exit(1);

}
