/*
 *  recorder_with_rendering.ino - Recording with rendering application
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
#include <MediaPlayer.h>
#include <OutputMixer.h>
#include <MemoryUtil.h>

#define RECORD_FILE_NAME "Sound.wav"

SDClass theSD;
File s_myFile;

MediaRecorder *theRecorder;
MediaPlayer *thePlayer;
OutputMixer *theMixer;

const int32_t sc_buffer_size = 6144;
uint8_t s_buffer[sc_buffer_size];
const int32_t sc_dummy_frame_num = 5;

bool ErrEnd = false;

/**
 * @brief Audio attention callback
 *
 * When audio internal error occurs, this function will be called back.
 */

static void attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");
  
  if (atprm->error_code > AS_ATTENTION_CODE_WARNING)
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
                                      AsOutputMixDoneParam *done_param)
{
  return;
}

/**
 * @brief Mixer data send callback procedure
 *
 * @param [in] identifier   Device identifier
 * @param [in] is_end       For normal request give false, for stop request give true
 */
static void outmixer_send_callback(int32_t identifier, bool is_end)
{
  AsRequestNextParam next;

  next.type = (!is_end) ? AsNextNormalRequest : AsNextStopResRequest;

  AS_RequestNextPlayerProcess(AS_PLAYER_ID_0, &next);

  return;
}

/**
 * @brief Player done callback procedure
 *
 * @param [in] event        AsPlayerEvent type indicator
 * @param [in] result       Result
 * @param [in] sub_result   Sub result
 *
 * @return true on success, false otherwise
 */
static bool mediaplayer_done_callback(AsPlayerEvent event, uint32_t result, uint32_t sub_result)
{
  /* If result of "Play", Start recording to supply captured data to player */

  if (event == AsPlayerEventPlay)
    {
      theRecorder->start();
    }

  return true;
}

/**
 * @brief Player decode callback procedure
 *
 * @param [in] pcm_param    AsPcmDataParam type
 */
void mediaplayer_decode_callback(AsPcmDataParam pcm_param)
{
  /* You can imprement any audio signal process */
   
  /* Output and sound audio data */
  theMixer->sendData(OutputMixer0,
                     outmixer_send_callback,
                     pcm_param);
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
  /* Initialize SD */
  while (!theSD.begin())
    {
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
    }

  /* Initialize memory pools and message libs */

  initMemoryPools();
  createStaticPools(MEM_LAYOUT_RECORDINGPLAYER);

  /* start audio system */

  theRecorder = MediaRecorder::getInstance();
  thePlayer = MediaPlayer::getInstance();
  theMixer  = OutputMixer::getInstance();

  theRecorder->begin();
  thePlayer->begin();
  theMixer->begin();

  puts("initialization MediaRecorder, MediaPlayer and OutputMixer");


  /* Create Objects */

  thePlayer->create(MediaPlayer::Player0, attention_cb);
  theMixer->create(attention_cb);

  /* Activate Objects. Set output device to Speakers/Headphones */

  theRecorder->activate(AS_SETRECDR_STS_INPUTDEVICE_MIC, mediarecorder_done_callback);
  thePlayer->activate(MediaPlayer::Player0, AS_SETPLAYER_OUTPUTDEVICE_SPHP, mediaplayer_done_callback);
  theMixer->activate(OutputMixer0, outputmixer_done_callback);

  usleep(100 * 1000);

  /*
   * Initialize recorder to decode stereo wav stream with 48kHz sample rate
   * Search for SRC filter in "/mnt/sd0/BIN" directory
   */

  theRecorder->init(AS_CODECTYPE_WAV, AS_CHANNEL_STEREO, AS_SAMPLINGRATE_48000, AS_BITLENGTH_16, AS_BITRATE_8000);
  puts("recorder init");
  thePlayer->init(MediaPlayer::Player0, AS_CODECTYPE_WAV, AS_SAMPLINGRATE_48000, AS_CHANNEL_STEREO);
  puts("player init");

  theMixer->setVolume(0, 0, 0);

  if (theSD.exists(RECORD_FILE_NAME))
    {
      printf("Remove existing file [%s].\n", RECORD_FILE_NAME);
      theSD.remove(RECORD_FILE_NAME);
    }

  /* Open file to record */
  
  s_myFile = theSD.open(RECORD_FILE_NAME, FILE_WRITE);

  printf("Open! [%s]\n", RECORD_FILE_NAME);

  /* Verify file open */
  
  if (!s_myFile)
    {
      printf("File open error\n");
      exit(1);
    }

  theRecorder->writeWavHeader(s_myFile);
  puts("Write Header!");
}

/**
 * @brief Record audio frames
 */

typedef enum
{
  StateReady = 0,
  StateRun,
} State;

void loop()
{
  static int s_cnt = 0;
  static State s_state = StateReady;

  if (s_state == StateReady)
    {
      /* Prestore audio dummy data to start player */

      memset(s_buffer, 0, sizeof(s_buffer));
      for (int i = 0; i < sc_dummy_frame_num; i++)
        {
          thePlayer->writeFrames(MediaPlayer::Player0, s_buffer, sizeof(s_buffer));
        }
      
      /* Start play */
      
      thePlayer->start(MediaPlayer::Player0, mediaplayer_decode_callback);
      puts("player start");

      s_state = StateRun;
    }
  else if (s_state == StateRun)
    {
      /* Get recorded data and play */

      uint32_t read_size = 0;

      do
        {
          theRecorder->readFrames(s_buffer, sc_buffer_size, &read_size);

          if (read_size > 0)
            {
              thePlayer->writeFrames(MediaPlayer::Player0, s_buffer, read_size);

              /* Write captured data to the file too */
              
              if (s_myFile.write((uint8_t*)s_buffer, read_size) < 0)
                {
                  puts("File write error!");
                  goto exitRecording;
                }
            }
        }
      while (read_size != 0);

      if (s_cnt++ > 400)
        {
          goto exitRecording;
        }
    }
  else
    {
    }

  if (ErrEnd)
    {
      printf("Error End\n");
      goto exitRecording;
    }

  /* The usleep() function suspends execution of the calling thread for usec
   * microseconds. But the timer resolution depends on the OS system tick time
   * which is 10 milliseconds (10,000 microseconds) by default. Therefore,
   * it will sleep for a longer time than the time requested here.
   */

  usleep(10 * 1000);

  return;

exitRecording:

  thePlayer->stop(MediaPlayer::Player0);
  theRecorder->stop();

  theRecorder->writeWavHeader(s_myFile);
  puts("Update Header!");
  
  s_myFile.close();

  thePlayer->deactivate(MediaPlayer::Player0);
  thePlayer->end();

  theRecorder->deactivate();
  theRecorder->end();

  puts("End Recording with rendering");

  exit(1);
}
