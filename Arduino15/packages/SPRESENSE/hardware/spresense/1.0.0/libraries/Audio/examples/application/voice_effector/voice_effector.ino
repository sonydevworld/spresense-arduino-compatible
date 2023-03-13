/*
 *  voice_effector.ino - Object I/F based effector application
 *  Copyright 2018,2021 Sony Semiconductor Solutions Corporation
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

#include <FrontEnd.h>
#include <OutputMixer.h>
#include <MemoryUtil.h>
#include <arch/board/board.h>

FrontEnd *theFrontEnd;
OutputMixer *theMixer;

static const int32_t channel_num  = AS_CHANNEL_STEREO;
static const int32_t bit_length   = AS_BITLENGTH_16;
static const int32_t frame_sample = 240;
static const int32_t frame_size   = frame_sample * (bit_length / 8) * channel_num;

static const int32_t proc_size  = frame_size;
static uint8_t proc_buffer[proc_size];

bool isCaptured = false;
bool isEnd = false;
bool ErrEnd = false;

/**
 * @brief Sample singnal Processing function
 *
 * @param [in] uint16_t   ptr
 * @param [in] int        size
 */
void signal_process(int16_t* ptr, int size)
{
  static int frame_cnt = 0;

  if (frame_cnt < 1000) {
    ledOn(LED0);
    rc_filter(ptr, size);
  } else if (frame_cnt < 2000) {
    ledOff(LED0);
    ledOn(LED1);
    distortion_filter(ptr, size);
  } else if (frame_cnt < 3000) {
    ledOff(LED1);
    ledOn(LED2);
    /* No filter */
  } else {
    ledOff(LED2);
    frame_cnt = 0;
  }

  frame_cnt++;
}

/**
 * @brief RC-Filter function
 *
 * @param [in] pcm_param    AsPcmDataParam type
 */
void rc_filter(int16_t* ptr, int size)
{
  /* Example : RC filter for 16bit PCM */

  static const int PeakLevel = 32700;
  static const int LevelGain = 2;

  int16_t *ls = (int16_t*)ptr;
  int16_t *rs = ls + 1;

  static int16_t ls_l = 0;
  static int16_t rs_l = 0;

  if (!ls_l && !rs_l) {
    ls_l = *ls * LevelGain;
    rs_l = *rs * LevelGain;
  }

  for (int cnt = 0; cnt < size; cnt += 4) {
    int32_t tmp;

    *ls = *ls * LevelGain;
    *rs = *rs * LevelGain;

    tmp = (ls_l * 98 / 100) + (*ls * 2 / 100);
    *ls = clip(tmp, PeakLevel);
    tmp = (rs_l * 98 / 100) + (*rs * 2 / 100);
    *rs = clip(tmp, PeakLevel);

    ls_l = *ls;
    rs_l = *rs;

    ls += 2;
    rs += 2;
  }
}

/**
 * @brief Distortion(Peak-cut)-Filter function
 *
 * @param [in] pcm_param    AsPcmDataParam type
 */
void distortion_filter(int16_t* ptr, int size)
{
  /* Example : Distortion filter for 16bit PCM */

  static const int PeakLevel = 170;

  int16_t *ls = ptr;
  int16_t *rs = ls + 1;

  for (int32_t cnt = 0; cnt < size; cnt += 4) {
    int32_t tmp;

    tmp = *ls * 4 / 3;
    *ls = clip(tmp, PeakLevel);
    tmp = *rs * 4 / 3;
    *rs = clip(tmp, PeakLevel);

    ls += 2;
    rs += 2;
  }
}

/**
 * @brief clip function
 *
 * @param [in] val   source value
 * @param [in] peak  clip point value
 */
inline int16_t clip(int32_t val, int32_t peak)
{
  return (val > 0) ? ((val < peak) ? val : peak) : ((val > (-1 * peak)) ? val : (-1 * peak));
}

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
 * @brief OutputMixer attention callback
 *
 * When audio internal error occurs, this function will be called back.
 */
void mixer_attention_cb(const ErrorAttentionParam *param)
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
 * @brief Mixer done callback procedure
 *
 * @param [in] requester_dtq    MsgQueId type
 * @param [in] reply_of         MsgType type
 * @param [in,out] done_param   AsOutputMixDoneParam type pointer
 */
static void outputmixer_done_callback(MsgQueId requester_dtq,
                                      MsgType reply_of,
                                      AsOutputMixDoneParam* done_param)
{
  UNUSED(requester_dtq);
  UNUSED(reply_of);
  UNUSED(done_param);
  return;
}

/**
 * @brief Pcm capture on FrontEnd callback procedure
 *
 * @param [in] pcm          PCM data structure
 */
static void frontend_pcm_callback(AsPcmDataParam pcm)
{
  if (!pcm.is_valid) {
    puts("Invalid data !");
    memset(proc_buffer, 0, frame_size);
  } else {
    if (pcm.size > frame_size) {
      puts("Capture size is too big!");
      pcm.size = frame_size;
    }

    if (pcm.size == 0) {
      memset(proc_buffer, 0, frame_size);        
    } else {
      memcpy(proc_buffer, pcm.mh.getPa(), pcm.size);
    }
  }

  if (pcm.is_end) {
    isEnd = true;
  }

  isCaptured = true;

  return;
}

/**
 * @brief Mixer data send callback procedure
 *
 * @param [in] identifier   Device identifier
 * @param [in] is_end       For normal request give false, for stop request give true
 */
static void outmixer0_send_callback(int32_t identifier, bool is_end)
{
  /* Do nothing, as the pcm data already sent in the main loop. */
  UNUSED(identifier);
  UNUSED(is_end);
  return;
}

/**
 * @brief Execute signal processing for one frame
 */
bool execute_aframe()
{
  isCaptured = false;
  signal_process((int16_t*)proc_buffer, proc_size);

  AsPcmDataParam pcm_param;

  /* Alloc MemHandle */
  while (pcm_param.mh.allocSeg(S0_REND_PCM_BUF_POOL, frame_size) != ERR_OK) {
    delay(1);
  }

  if (isEnd) {
    pcm_param.is_end = true;
  } else {
    pcm_param.is_end = false;
  }

  /* Set PCM parameters */
  pcm_param.identifier = OutputMixer0;
  pcm_param.callback = 0;
  pcm_param.bit_length = bit_length;
  pcm_param.size = frame_size;
  pcm_param.sample = frame_sample;
  pcm_param.is_valid = true;

  memcpy(pcm_param.mh.getPa(), proc_buffer, pcm_param.size);

  int err = theMixer->sendData(OutputMixer0,
                               outmixer0_send_callback,
                               pcm_param);

  if (err != OUTPUTMIXER_ECODE_OK) {
    printf("OutputMixer send error: %d\n", err);
    return false;
  }

  return true;

}

/**
 * @brief Setup Audio Objects
 *
 * Set input device to Mic <br>
 * Initialize frontend to capture stereo and 48kHz sample rate <br>
 */
void setup()
{
  /* Initialize serial */
  Serial.begin(115200);
  while (!Serial);

  /* Initialize memory pools and message libs */
  initMemoryPools();
  createStaticPools(MEM_LAYOUT_RECORDINGPLAYER);

  /* Clear the buffer for singnal processing */
  memset(proc_buffer, 0, proc_size);

  /* Begin objects */
  theFrontEnd = FrontEnd::getInstance();
  theMixer = OutputMixer::getInstance();

  theFrontEnd->begin(frontend_attention_cb);
  theMixer->begin();

  puts("begin FrontEnd and OutputMixer");

  /* Create Objects */
  theMixer->create(mixer_attention_cb);

  /* Set capture clock */
  theFrontEnd->setCapturingClkMode(FRONTEND_CAPCLK_NORMAL);

  /* Activate objects */
  theFrontEnd->activate(frontend_done_callback);
  theMixer->activate(OutputMixer0, outputmixer_done_callback);

  usleep(100 * 1000); /* waiting for Mic startup */

  /* Initialize each objects */
  AsDataDest dst;
  dst.cb = frontend_pcm_callback;
  theFrontEnd->init(channel_num, bit_length, frame_sample, AsDataPathCallback, dst);

  /* Set rendering volume */
  theMixer->setVolume(0, 0, 0);

  /* Unmute */
  board_external_amp_mute_control(false);

  theFrontEnd->start();

}

/**
 * @brief audio loop
 */
void loop()
{
  if (ErrEnd) {
    puts("Error End");
    theFrontEnd->stop();
    goto exitCapturing;
  }

  if (isCaptured) {
    if (!execute_aframe()) {
      printf("Rendering error!\n");
      goto exitCapturing;
    } 
  }

  if (isEnd && !isCaptured) {
    isEnd= false;
    goto exitCapturing;
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

  return;

exitCapturing:
  board_external_amp_mute_control(true); 
  theFrontEnd->deactivate();
  theMixer->deactivate(OutputMixer0);
  theFrontEnd->end();
  theMixer->end();

  puts("Exit.");
  exit(1);
}
