/*
 *  MainAudio.ino - IIR Example with Audio (Low Pass Filter)
 *  Copyright 2019, 2021 Sony Semiconductor Solutions Corporation
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

#include <MP.h>

#include <FrontEnd.h>
#include <OutputMixer.h>
#include <MemoryUtil.h>
#include <arch/board/board.h>

FrontEnd *theFrontEnd;
OutputMixer *theMixer;

/* Setting audio parameters */
static const int32_t channel_num  = AS_CHANNEL_MONO;
static const int32_t bit_length   = AS_BITLENGTH_16;
static const int32_t frame_sample = 768;
static const int32_t capture_size = frame_sample * (bit_length / 8) * channel_num;
static const int32_t render_size  = frame_sample * (bit_length / 8) * 2;


/* Multi-core parameters */
const int subcore = 1;

struct Request {
  void *buffer;
  int  sample;
};

struct Result {
  void *buffer;
  int  sample;
};

/* Application flags */
bool isCaptured = false;
bool isEnd = false;
bool ErrEnd = false;

/**
 * @brief Frontend attention callback
 *
 * When audio internal error occurc, this function will be called back.
 */

static void frontend_attention_cb(const ErrorAttentionParam *param)
{
  puts("Attention!");

  if (param->error_code >= AS_ATTENTION_CODE_WARNING) {
    ErrEnd = true;
  }
}

/**
 * @brief OutputMixer attention callback
 *
 * When audio internal error occurc, this function will be called back.
 */
static void mixer_attention_cb(const ErrorAttentionParam *param)
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
  int8_t sndid = 100; /* user-defined msgid */
  static Request request;

  if (pcm.size > capture_size) {
    puts("Capture size is too big!");
    pcm.size = capture_size;
  }

  request.buffer  = pcm.mh.getPa();
  request.sample  = pcm.size / (bit_length / 8) / channel_num;

  if (!pcm.is_valid) {
    puts("Invalid data !");
    memset(request.buffer , 0, request.sample);
  }

  MP.Send(sndid, &request, subcore);

  if (pcm.is_end) {
    isEnd = true;
  }

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

static bool send_mixer(Result* res)
{
  AsPcmDataParam pcm_param;

  /* Alloc MemHandle */
  while (pcm_param.mh.allocSeg(S0_REND_PCM_BUF_POOL, render_size) != ERR_OK) {
    delay(1);
  }

  if (isEnd) {
    pcm_param.is_end = true;
    isEnd = false;
  } else {
    pcm_param.is_end = false;
  }

  /* Set PCM parameters */
  pcm_param.identifier = OutputMixer0;
  pcm_param.callback = 0;
  pcm_param.bit_length = bit_length;
  pcm_param.size = render_size;
  pcm_param.sample = frame_sample;
  pcm_param.is_valid = true;

  memcpy(pcm_param.mh.getPa(), res->buffer, pcm_param.size);

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
 * @brief Setup Audio & MP objects
 */
void setup()
{
  /* Initialize serial */
  Serial.begin(115200);
  while (!Serial);

  /* Launch SubCore */
  int ret = MP.begin(subcore);
  if (ret < 0) {
    printf("MP.begin error = %d\n", ret);
  }

  /* receive with non-blocking */
  //  MP.RecvTimeout(20);

  /* Initialize memory pools and message libs */
  Serial.println("Init memory Library");

  initMemoryPools();
  createStaticPools(MEM_LAYOUT_RECORDINGPLAYER);

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
  theMixer->setVolume(-6, 0, 0);

  /* Unmute */
  board_external_amp_mute_control(false);

  theFrontEnd->start();

}

/**
 * @brief Audio loop
 */
void loop()
{
  int8_t   rcvid = 0;
  static Result*  result;

  /* Receive sound from SubCore */
  int ret = MP.Recv(&rcvid, &result, subcore);

  if (ret < 0) {
    printf("MP error! %d\n", ret);
    return;
  }

  if (result->sample != frame_sample) {
    printf("Size miss match.%d,%ld\n", result->sample, frame_sample);
    goto exitCapturing;
  }

  if (!send_mixer(result)) {
    printf("Rendering error!\n");
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
