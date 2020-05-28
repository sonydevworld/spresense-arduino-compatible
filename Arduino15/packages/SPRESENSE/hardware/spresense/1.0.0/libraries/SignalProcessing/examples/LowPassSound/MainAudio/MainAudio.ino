/*
 *  MainAudio.ino - IIR Example with Audio (Low Pass Filter)
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

#include <MP.h>
#include <MediaRecorder.h>
#include <MediaPlayer.h>
#include <OutputMixer.h>
#include <MemoryUtil.h>

MediaRecorder *theRecorder;
MediaPlayer *thePlayer;
OutputMixer *theMixer;

/* Now suppot only mono channel*/

/* Select mic channel number */
const int mic_channel_num = 1;
//const int mic_channel_num = 2;

const int32_t s_buffer_size = 768 * mic_channel_num * sizeof(uint16_t);
uint8_t s_buffer[s_buffer_size];
bool ErrEnd = false;

const int subcore = 1;

struct Request {
  void *buffer;
  int  sample;
};

struct Result {
  void *buffer;
  int  sample;
};

/**
   @brief Audio attention callback

   When audio internal error occurc, this function will be called back.
*/

static void attention_callback(const ErrorAttentionParam *atprm)
{
  puts("Attention!");

  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
  {
    ErrEnd = true;
  }
}

/**
   @brief Recorder done callback procedure

   @param [in] event        AsRecorderEvent type indicator
   @param [in] result       Result
   @param [in] sub_result   Sub result

   @return true on success, false otherwise
*/

static bool mediarecorder_event_callback(AsRecorderEvent event, uint32_t result, uint32_t sub_result)
{
  return true;
}

/**
   @brief Mixer done callback procedure

   @param [in] requester_dtq    MsgQueId type
   @param [in] reply_of         MsgType type
   @param [in,out] done_param   AsOutputMixDoneParam type pointer
*/
static void outputmixer_event_callback(MsgQueId requester_dtq,
                                      MsgType reply_of,
                                      AsOutputMixDoneParam *done_param)
{
  return;
}

/**
   @brief Mixer data send callback procedure

   @param [in] identifier   Device identifier
   @param [in] is_end       For normal request give false, for stop request give true
*/
static void outmixer_rendering_callback(int32_t identifier, bool is_end)
{
  AsRequestNextParam next;

  next.type = (!is_end) ? AsNextNormalRequest : AsNextStopResRequest;

  AS_RequestNextPlayerProcess(AS_PLAYER_ID_0, &next);

  return;
}

/**
   @brief Player done callback procedure

   @param [in] event        AsPlayerEvent type indicator
   @param [in] result       Result
   @param [in] sub_result   Sub result

   @return true on success, false otherwise
*/
static bool mediaplayer_event_callback(AsPlayerEvent event, uint32_t result, uint32_t sub_result)
{
  /* If result of "Play", restart recording (It should been stopped when "Play" requested) */
  return true;
}

/**
   @brief Player decode callback procedure

   @param [in] pcm_param    AsPcmDataParam type
*/
void mediaplayer_decode_callback(AsPcmDataParam pcm_param)
{
  /* You can imprement any audio signal process */

  /* Output and sound audio data */
  theMixer->sendData(OutputMixer0,
                     outmixer_rendering_callback,
                     pcm_param);
}

void setup()
{

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

  /* start audio system */
  Serial.println("Init Audio Library");

  theRecorder = MediaRecorder::getInstance();
  thePlayer = MediaPlayer::getInstance();
  theMixer  = OutputMixer::getInstance();

  theRecorder->begin();
  thePlayer->begin();

  puts("initialization MediaRecorder, MediaPlayer and OutputMixer");

  theMixer->activateBaseband();

  /* Create Objects */

  thePlayer->create(MediaPlayer::Player0, attention_callback);
  theMixer->create(attention_callback);

  /* Activate Objects. Set output device to Speakers/Headphones */

  theRecorder->activate(AS_SETRECDR_STS_INPUTDEVICE_MIC, mediarecorder_event_callback);
  thePlayer->activate(MediaPlayer::Player0, AS_SETPLAYER_OUTPUTDEVICE_SPHP, mediaplayer_event_callback);
  theMixer->activate(OutputMixer0, outputmixer_event_callback);

  usleep(100 * 1000);

  /*
     Initialize recorder to decode stereo wav stream with 48kHz sample rate
     Search for SRC filter in "/mnt/sd0/BIN" directory
  */
  uint8_t channel;
  switch (mic_channel_num) {
    case 1: channel = AS_CHANNEL_MONO;   break;
    case 2: channel = AS_CHANNEL_STEREO; break;
    case 4: channel = AS_CHANNEL_4CH;    break;
  }

  theRecorder->init(AS_CODECTYPE_LPCM, channel, AS_SAMPLINGRATE_48000, AS_BITLENGTH_16, 0);
  theRecorder->setMicGain(180);
  puts("recorder init");
  thePlayer->init(MediaPlayer::Player0, AS_CODECTYPE_WAV, AS_SAMPLINGRATE_48000, channel);
  puts("player init");

  /* Start Recorder */

  theMixer->setVolume(0, 0, 0);

}

typedef enum e_appState {
  StateReady = 0,
  StatePrepare,
  StateRun
} appState_t;

void loop()
{
  static appState_t state = StateReady;

  int8_t   sndid = 100; /* user-defined msgid */
  int8_t   rcvid = 0;
  int      read_size;
  int      ret;

  static Request  request;
  static Result*  result;

  switch (state) {
    case StateReady:
      {
      /* Start recording */
      theRecorder->start();
      puts("recorder start");
      state = StatePrepare;
      break;
      }
    case StatePrepare:
    {
      static int pre_cnt = 0;
      err_t err = theRecorder->readFrames(s_buffer, s_buffer_size, &read_size);

      if (err != 0) { printf("Error! %x ,%x , %d\n",err,request.buffer,request.sample); }

      if (read_size > 0)
      {
        request.buffer  = s_buffer;
        request.sample = read_size/ sizeof(uint16_t) / mic_channel_num;
        MP.Send(sndid, &request, subcore);

        if (pre_cnt < 2) {
          pre_cnt++;
          break;
        }

        /* Receive sound from SubCore */
        int ret = MP.Recv(&rcvid, &result, subcore);
        if (ret >= 0) {
          if (result->sample != 768) { break;}
          thePlayer->writeFrames(MediaPlayer::Player0, result->buffer, result->sample*2);
          pre_cnt++;
        } else {
          printf("error! %d", ret);
          exit(1);
        }
      } else {
        break;
      }

      if (pre_cnt > 10) {
        puts("player start");
        thePlayer->start(MediaPlayer::Player0, mediaplayer_decode_callback);
        pre_cnt = 0;
        state = StateRun;
      }
      break;
    }

    case StateRun:
    {
      err_t err = theRecorder->readFrames(s_buffer, s_buffer_size, &read_size);

      if (err != 0) {printf("Error! %x ,%x , %d\n",err,request.buffer,read_size);}
      if (read_size > 0)
      {
        request.buffer  = s_buffer;
        request.sample = read_size / sizeof(uint16_t) / mic_channel_num;
        MP.Send(sndid, &request, subcore);

        /* Receive sound from SubCore */
        ret = MP.Recv(&rcvid, &result, subcore);
        if (ret >= 0) {
          if (result->sample != 768) {break;}
          thePlayer->writeFrames(MediaPlayer::Player0,result->buffer, result->sample*2);
        } else {
          printf("error! %d", ret);
          exit(1);
        }
      } else {
        break;
      }
      break;
    }
    default:
    {
      puts("error!");
      exit(1);
    }
  }
}
