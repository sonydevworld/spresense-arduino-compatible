/*
 *  pcm_capture_objif.ino - PCM capture using object if example application
 *  Copyright 2018, 2021 Sony Semiconductor Solutions Corporation
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

#include <string.h>

#include <FrontEnd.h>
#include <MemoryUtil.h>

FrontEnd *theFrontEnd;

static const int32_t channel_num  = AS_CHANNEL_4CH;
static const int32_t bit_length   = AS_BITLENGTH_16;
static const int32_t frame_sample = 384;
static const int32_t frame_size   = frame_sample * (bit_length / 8) * channel_num;

static CMN_SimpleFifoHandle simple_fifo_handle;
static const int32_t fifo_size  = frame_size * 10;
static uint32_t fifo_buffer[fifo_size / sizeof(uint32_t)];


static const int32_t proc_size  = frame_size * 2;
static uint8_t proc_buffer[proc_size];

bool ErrEnd = false;

/**
 * @brief Frontend attention callback
 *
 * When audio internal error occurs, this function will be called back.
 */

void frontend_attention_cb(const ErrorAttentionParam *param)
{
  puts("Attention!");

  if (param->error_code >= AS_ATTENTION_CODE_WARNING) {
    ErrEnd = true;
  }
}

/**
 * @brief Frontend done callback procedure
 *
 * @param [in] event        AsMicFrontendEvent type indicator
 * @param [in] result       Result
 * @param [in] sub_result   Sub result
 *
 * @return true on success, false otherwise
 */

static bool frontend_done_callback(AsMicFrontendEvent ev, uint32_t result, uint32_t sub_result)
{
  UNUSED(ev);
  UNUSED(result);
  UNUSED(sub_result);
  return true;
}

/**
 * @brief Frontend pcm capture callback procedure
 *
 * @param [in] pcm          PCM data structure
 *
 * @return void
 */
static void frontend_pcm_callback(AsPcmDataParam pcm)
{
  if (!pcm.is_valid) {
    puts("Invalid data !");
    return;
  }

  if (CMN_SimpleFifoGetVacantSize(&simple_fifo_handle) < pcm.size) {
    puts("Simple FIFO is full !");
    return;
  }

  if (CMN_SimpleFifoOffer(&simple_fifo_handle, (const void*)(pcm.mh.getPa()), pcm.size) == 0) {
    puts("Simple FIFO is full !");
    return;
  }

  return;
}

/**
 *  @brief Setup audio device to capture PCM stream
 *
 *  Select input device as microphone <br>
 *  Set PCM capture sapling rate parameters to 48 kb/s <br>
 *  Set channel number 4 to capture audio from 4 microphones simultaneously <br>
 */
void setup()
{
  /* Initialize memory pools and message libs */
  initMemoryPools();
  createStaticPools(MEM_LAYOUT_RECORDER);


  if (CMN_SimpleFifoInitialize(&simple_fifo_handle, fifo_buffer, fifo_size, NULL) != 0) {
    print_err("Fail to initialize simple FIFO.\n");
    exit(1);
  }

  /* start audio system */
  theFrontEnd = FrontEnd::getInstance();
  theFrontEnd->begin(frontend_attention_cb);

  puts("initialization FrontEnd");

  /* Set capture clock */
  theFrontEnd->setCapturingClkMode(FRONTEND_CAPCLK_NORMAL);

  /* Activate Objects. Set output device to Microphone */
  theFrontEnd->activate(frontend_done_callback);

  usleep(100 * 1000); /* waiting for Mic startup */

  /* Initialize of capture */
  AsDataDest dst;
  dst.cb = frontend_pcm_callback;

  theFrontEnd->init(channel_num,
                    bit_length,
                    frame_sample,
                    AsDataPathCallback,
                    dst);

  theFrontEnd->start();
  puts("Capturing Start!");

}

/**
 * @brief Audio signal process (Modify for your application)
 */
void signal_process(uint32_t size)
{
  /* This is just a sample. 
     Please write what you want to process. */

  printf("Size %ld [%02x %02x %02x %02x %02x %02x %02x %02x ...]\n",
         size,
         proc_buffer[0],
         proc_buffer[1],
         proc_buffer[2],
         proc_buffer[3],
         proc_buffer[4],
         proc_buffer[5],
         proc_buffer[6],
         proc_buffer[7]);
}

/**
 * @brief Capture a frame of PCM data into buffer for signal processing
 */
bool execute_aframe()
{
  size_t size = CMN_SimpleFifoGetOccupiedSize(&simple_fifo_handle);

  if (size > 0) {
    if (size > proc_size) {
      size = (size_t)proc_size;
    }

    if (CMN_SimpleFifoPoll(&simple_fifo_handle, (void*)proc_buffer, size) == 0) {
      printf("ERROR: Fail to get data from simple FIFO.\n");
      return false;
    }

    signal_process(size);

  }

  return true;
}

/**
 * @brief Main loop
 */

void loop() {

  /* Execute audio data */
  if (!execute_aframe()) {
    puts("Capturing Error!");
    ErrEnd = true;
  }

  /* This sleep is adjusted by the time to write the audio stream file.
   * Please adjust in according with the processing contents
   * being processed at the same time by Application.
   *
   * The usleep() function suspends execution of the calling thread for usec microseconds.
   * But the timer resolution depends on the OS system tick time 
   * which is 10 milliseconds (10,000 microseconds) by default.
   * Therefore, it will sleep for a longer time than the time requested here.
   */

  //  usleep(10 * 1000);

  if (ErrEnd) {
    puts("Error End");
    theFrontEnd->stop();
    goto exitCapturing;
  }

  return;

exitCapturing:
  theFrontEnd->deactivate();
  theFrontEnd->end();

  puts("End Capturing");
  exit(1);

}
