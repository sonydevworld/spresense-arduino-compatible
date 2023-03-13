/*
 *  dual_players_objif.ino - Object I/F based dual source sound player
 *
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

#include <SDHCI.h>
#include <MediaPlayer.h>
#include <OutputMixer.h>
#include <MemoryUtil.h>

/* Content number 1 information definition. */

#define PLAYBACK0_FILE_NAME    "Sound0.mp3"
#define PLAYBACK0_CH_NUM        AS_CHANNEL_STEREO
#define PLAYBACK0_SAMPLING_RATE AS_SAMPLINGRATE_AUTO
#define PLAYBACK0_CODEC_TYPE    AS_CODECTYPE_MP3

/* Content number 2 information definition. */

#define PLAYBACK1_FILE_NAME    "Sound0.wav"
#define PLAYBACK1_CH_NUM        AS_CHANNEL_STEREO
#define PLAYBACK1_SAMPLING_RATE AS_SAMPLINGRATE_48000
#define PLAYBACK1_CODEC_TYPE    AS_CODECTYPE_WAV

/* Set volume[db] */

#define VOLUME_MASTER     -160
#define VOLUME_PLAY0      -160
#define VOLUME_PLAY1      -160

/* Play element */

static struct PLAY_ELM_T
{
  MediaPlayer::PlayerId play_id;
  sem_t                 sem;
  bool                  err_end;
  DecodeDoneCallback    decode_callback;
  const char           *file_name;
  uint8_t               codec_type;
  uint32_t              sampling_rate;
  uint8_t               channel_number;
}
play0_elm,
play1_elm;

SDClass theSD;

/**
 * @brief Audio(player0) attention callback
 *
 * When audio internal error occurs, this function will be called back.
 */

static void attention_player_cb(const ErrorAttentionParam *atprm)
{
  printf("Attention(player%d)!\n", atprm->module_id);

  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      if (atprm->module_id == AS_PLAYER_ID_0)
        {
          play0_elm.err_end = true;
        }
      else
        {
          play1_elm.err_end = true;
        }
   }
}

/**
 * @brief Audio(mixer) attention callback
 *
 */

static void attention_mixer_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention(mixer)!");

  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      play0_elm.err_end = true;
      play1_elm.err_end = true;
   }
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
 * @brief Mixer0 data send callback procedure
 *
 * @param [in] identifier   Device identifier
 * @param [in] is_end       For normal request give false, for stop request give true
 */

static void outmixer0_send_callback(int32_t identifier, bool is_end)
{
  AsRequestNextParam next;

  next.type = (!is_end) ? AsNextNormalRequest : AsNextStopResRequest;

  AS_RequestNextPlayerProcess(AS_PLAYER_ID_0, &next);

  return;
}

/**
 * @brief Mixer1 data send callback procedure
 *
 */

static void outmixer1_send_callback(int32_t identifier, bool is_end)
{
  AsRequestNextParam next;

  next.type = (!is_end) ? AsNextNormalRequest : AsNextStopResRequest;

  AS_RequestNextPlayerProcess(AS_PLAYER_ID_1, &next);

  return;
}

/**
 * @brief Player0 ad Player1 done callback procedure
 *
 * @param [in] event        AsPlayerEvent type indicator
 * @param [in] result       Result
 * @param [in] sub_result   Sub result
 *
 * @return true on success, false otherwise
 */

static bool mediaplayer_done_callback0(AsPlayerEvent event, uint32_t result, uint32_t sub_result)
{
  sem_post(&play0_elm.sem);

  return true;
}

/**
 * @brief Player1 ad Player1 done callback procedure
 *
 */

static bool mediaplayer_done_callback1(AsPlayerEvent event, uint32_t result, uint32_t sub_result)
{
  sem_post(&play1_elm.sem);

  return true;
}

/**
 * @brief Player0 decode callback procedure
 *
 * @param [in] pcm_param    AsPcmDataParam type
 */

void mediaplayer0_decode_callback(AsPcmDataParam pcm_param)
{
  {
    /* Decoded PCM data can be processed here. */
    
    signed short *ptr = (signed short *)pcm_param.mh.getPa();

    for (unsigned int cnt = 0; cnt < pcm_param.size; cnt += 4)
      {
        *ptr = *ptr + 0;
        ptr++;
        *ptr = *ptr + 0;
        ptr++;
      }
  }

  OutputMixer *theMixer  = OutputMixer::getInstance();

  theMixer->sendData(OutputMixer0,
                     outmixer0_send_callback,
                     pcm_param);
}

/**
 * @brief Player1 decode callback procedure
 *
 */

void mediaplayer1_decode_callback(AsPcmDataParam pcm_param)
{
  OutputMixer *theMixer  = OutputMixer::getInstance();

  theMixer->sendData(OutputMixer1,
                     outmixer1_send_callback,
                     pcm_param);
}

/**
 *  @brief Repeated transfer of data read into the decoder
 */

void play_process( MediaPlayer *thePlayer, struct PLAY_ELM_T *elm, File& file)
{
  while (1)
    {
      /* Send new frames to decode in a loop until file ends */

      err_t err = thePlayer->writeFrames(elm->play_id, file);

      /*  Tell when player file ends */

      if (err == MEDIAPLAYER_ECODE_FILEEND)
        {
          printf("Player%d File End!\n", elm->play_id);
          break;
        }

      /* Show error code from player and stop */

      else if (err != MEDIAPLAYER_ECODE_OK)
        {
          printf("Player%d error code: %d\n", elm->play_id, err);
          break;
        }

      if (elm->err_end)
        {
          printf("Error End\n");
          break;
        }

      /* This sleep is adjusted by the time to read the audio stream file.
       * Please adjust in according with the processing contents
       * being processed at the same time by Application.
       *
       * The usleep() function suspends execution of the calling thread for usec
       * microseconds. But the timer resolution depends on the OS system tick time
       * which is 10 milliseconds (10,000 microseconds) by default. Therefore,
       * it will sleep for a longer time than the time requested here.
       */

      usleep(40000);
    }
}

/**
 *  @brief Setup main audio player and sub audio player
 *
 *  Set output device to speaker <br>
 *  Open two stream files <br>
 *  These should be in the root directory of the SD card. <br>
 *  Set main player to decode stereo mp3. Stream sample rate is set to "auto detect" <br>
 *  System directory "/mnt/sd0/BIN" will be searched for MP3 decoder (MP3DEC file) <br>
 *  This is the /BIN directory on the SD card. <br>
 */

static int player_thread(int argc, FAR char *argv[])
{
  File               file;
  struct PLAY_ELM_T *elm;

  /* Get static audio modules instance */

  MediaPlayer *thePlayer = MediaPlayer::getInstance();

  /* Get information by task */

  elm = (struct PLAY_ELM_T *)strtol(argv[1], NULL, 16);

  printf("\"%s\" task start\n", argv[0]);

  /* Continue playing the same file. */

  while (1)
    {
      /*
       * Initialize main player to decode stereo mp3 stream with 48 kb/s sample rate
       * Search for MP3 codec in "/mnt/sd0/BIN" directory
       */

      err_t err = thePlayer->init(elm->play_id, elm->codec_type, "/mnt/sd0/BIN", elm->sampling_rate, elm->channel_number);

      if (err != MEDIAPLAYER_ECODE_OK)
        {
          printf("Initialize Error!=%d\n",err);
          file.close();
          break;
        }

      sem_wait(&elm->sem);

      file = theSD.open(elm->file_name);

      /* Verify file open */

      if (!file)
        {
          printf("File open error \"%s\"\n", elm->file_name);
          break;
        }

      printf("Open! %s\n", elm->file_name);

      /* Send first frames to be decoded */

      err = thePlayer->writeFrames(elm->play_id, file);

      if (err != MEDIAPLAYER_ECODE_OK && err != MEDIAPLAYER_ECODE_FILEEND)
        {
          printf("File Read Error! =%d\n",err);
          file.close();
          break;
        }

      /* Start Player */

      thePlayer->start(elm->play_id, elm->decode_callback);

      sem_wait(&elm->sem);

      printf("Start player%d!\n", elm->play_id);

      /* Running... */

      play_process(thePlayer, elm, file);

      /* Stop Player */

      printf("Stop player%d!\n", elm->play_id);

      thePlayer->stop(elm->play_id);

      sem_wait(&elm->sem);

      file.close();
    }

  printf("Exit task(%d).\n", elm->play_id);

  return 0;
}

void setup()
{
  /* Set player0 paramater */

  play0_elm.play_id         = MediaPlayer::Player0;
  play0_elm.err_end         = false;
  play0_elm.decode_callback = mediaplayer0_decode_callback;
  play0_elm.file_name       = PLAYBACK0_FILE_NAME;
  play0_elm.codec_type      = PLAYBACK0_CODEC_TYPE;
  play0_elm.sampling_rate   = PLAYBACK0_SAMPLING_RATE;
  play0_elm.channel_number  = PLAYBACK0_CH_NUM;
  sem_init(&play0_elm.sem, 0, 0);

  /* Set player1 paramater */

  play1_elm.play_id         = MediaPlayer::Player1;
  play1_elm.err_end         = false;
  play1_elm.decode_callback = mediaplayer1_decode_callback;
  play1_elm.file_name       = PLAYBACK1_FILE_NAME;
  play1_elm.codec_type      = PLAYBACK1_CODEC_TYPE;
  play1_elm.sampling_rate   = PLAYBACK1_SAMPLING_RATE;
  play1_elm.channel_number  = PLAYBACK1_CH_NUM;
  sem_init(&play1_elm.sem, 0, 0);

  /* Mount SD card */

  while (!theSD.begin())
    {
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
    }

  /* Initialize memory pools and message libs */

  initMemoryPools();
  createStaticPools(MEM_LAYOUT_PLAYER);

  /* Get static audio modules instance */

  MediaPlayer *thePlayer = MediaPlayer::getInstance();
  OutputMixer *theMixer  = OutputMixer::getInstance();

  /* start audio system */

  thePlayer->begin();

  puts("initialization Audio Library");

  /* Activate Baseband */

  theMixer->activateBaseband();

  /* Create Objects */

  thePlayer->create(MediaPlayer::Player0, attention_player_cb);
  thePlayer->create(MediaPlayer::Player1, attention_player_cb);

  theMixer->create(attention_mixer_cb);

  /* Set rendering clock */

  theMixer->setRenderingClkMode(OUTPUTMIXER_RNDCLK_NORMAL);

  /* Activate Player Object */

  thePlayer->activate(MediaPlayer::Player0, mediaplayer_done_callback0);
  sem_wait(&play0_elm.sem);
  thePlayer->activate(MediaPlayer::Player1, mediaplayer_done_callback1);
  sem_wait(&play1_elm.sem);

  /* Activate Mixer Object.
   * Set output device to speaker with 2nd argument.
   * If you want to change the output device to I2S,
   * specify "I2SOutputDevice" as an argument.
   */

  theMixer->activate(OutputMixer0, HPOutputDevice, outputmixer_done_callback);
  theMixer->activate(OutputMixer1, HPOutputDevice, outputmixer_done_callback);

  /* Set master volume, Player0 volume, Player1 volume */

  theMixer->setVolume(VOLUME_MASTER, VOLUME_PLAY0, VOLUME_PLAY1);

  /* Initialize task parameter. */

  const char *argv0[2];
  const char *argv1[2];
  char        param0[16];
  char        param1[16];

  snprintf(param0, 16, "%lx", (uint32_t)&play0_elm);
  snprintf(param1, 16, "%lx", (uint32_t)&play1_elm);

  argv0[0] = param0;
  argv0[1] = NULL;
  argv1[0] = param1;
  argv1[1] = NULL;

  /* Start task */

  task_create("player_thread0", 155, 2048, player_thread, (char* const*)argv0);
  task_create("player_thread1", 155, 2048, player_thread, (char* const*)argv1);
}

/**
 *  @brief Play streams
 *
 * Send new frames to decode in a loop until file ends
 */

void loop()
{
  /* Do nothing on main task. */

  sleep(1);
}
