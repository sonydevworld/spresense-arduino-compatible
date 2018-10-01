/*
 *  MediaPlayer.h - Audio include file for the Spresense SDK
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

#ifndef MediaPlayer_h
#define MediaPlayer_h

#include <SDHCI.h>

// #ifdef __cplusplus

#include <audio/audio_high_level_api.h>
#include <memutils/simple_fifo/CMN_SimpleFifo.h>

/*--------------------------------------------------------------------------*/

/**
 * MediaPlayer log output definition
 */
#define MEDIAPLAYER_LOG_DEBUG

#define print_err printf

#ifdef MEDIAPLAYER_LOG_DEBUG
#define print_dbg(...) printf(__VA_ARGS__)
#else
#define print_dbg(x...)
#endif

/**
 * MediaPlayer Error Code Definitions.
 */

#define MEDIAPLAYER_ECODE_OK            0
#define MEDIAPLAYER_ECODE_COMMAND_ERROR 1
#define MEDIAPLAYER_ECODE_SIMPLEFIFO_ERROR 2
#define MEDIAPLAYER_ECODE_FILEACCESS_ERROR 3
#define MEDIAPLAYER_ECODE_FILEEND 4
#define MEDIAPLAYER_ECODE_SHARED_MEMORY_ERROR 5

#define MEDIAPLAYER_BUF_FRAME_NUM  8
#define MEDIAPLAYER_BUF_FRAME_SIZE 6144
#define MEDIAPLAYER_BUF_SIZE (MEDIAPLAYER_BUF_FRAME_NUM * MEDIAPLAYER_BUF_FRAME_SIZE)

/*--------------------------------------------------------------------------*/

/**
 * @class MediaPlayer
 * @brief MediaPlayer Class Definitions.
 */

class MediaPlayer
{
public:

  /**
   * @brief Get instance of MediaRecorder for singleton.
   */

  static MediaPlayer* getInstance()
    {
      static MediaPlayer instance;
      return &instance;
    }

   /**
   * @enum Player ID
   *
   * @brief Audio library allows you to use two players simultaneously.
   *        Please set Player ID that player instance id created to use.
   */

  typedef enum
  {
    Player0,  /**< Indicates Player0 */
    Player1   /**< Indicates Player1 */
  } PlayerId;

  /**
   * @brief Initialize the MediaPlayer.
   *
   * @details This function is called only once when using the MediaPlayer.
   *          In this function, alloc memory area of FIFO for ES data supply.
   *
   */

  err_t begin(void);

  /**
   * @brief Creation of the MediaPlayer.
   *
   * @details This function is called only once when using the MediaPlayer.
   *          In this function, create objcets for audio data decoding.
   *
   */

  err_t create(
      PlayerId id /**< Select Player ID. */
  );

  /**
   * @brief Activate the MediaPlayer
   *
   * @details This function activates media player system.
   *          You should specify output device which you would like to sound.
   *          You can set "Speaker/HeadPhone-out" or "I2S-out".
   *          The result of APIs will be returnd by callback function which is
   *          specified by this function.
   *
   */

  err_t activate(
      PlayerId id,             /**< Select Player ID. */
      uint8_t output_device,   /**< Set output device. AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT*/
      MediaPlayerCallback mpcb /**< Sepcify callback function which is called to notify API results. */
  );

  /**
   * @brief Initialize the MediaPlayer (Abridged version).
   *
   * @details This is abridged version on initialize API.
   *          You can init media player with codec type, sampling rate, and number of channels.
   *          When this API is called, other parameters are fixed as below.
   *
   *          Bit length      : 16bit
   *          DSP binary path : /mnt/sd0/BIN
   *
   */

  err_t init(
      PlayerId id,            /**< Select Player ID. */
      uint8_t codec_type,     /**< Set compression code. AS_CODECTYPE_MP3 or AS_CODECTYPE_WAV */
      uint32_t sampling_rate, /**< Set sampling rate. AS_SAMPLINGRATE_XXXXX */
      uint8_t channel_number  /**< Set channnel number. AS_CHANNEL_MONO or AS_CHANNEL_STEREO */
  );

  /**
   * @brief Initialize the MediaPlayer (Abridged version).
   *
   * @details This is abridged version on initialize API.
   *          If you would like to set all of decode parameters but DSP binary path can be default,
   *          you can call this API. DSP binary path will be defautl as below.
   *
   *          DSP binary path : /mnt/sd0/BIN
   *
   */

  err_t init(
      PlayerId id,            /**< Select Player ID. */
      uint8_t codec_type,     /**< Set compression code. AS_CODECTYPE_MP3 or AS_CODECTYPE_WAV */
      uint32_t sampling_rate, /**< Set sampling rate. AS_SAMPLINGRATE_XXXXX */
      uint8_t bit_length,     /**< Set bit length. AS_BITLENGTH_16 or AS_BITLENGTH_24 */
      uint8_t channel_number  /**< Set channnel number. AS_CHANNEL_MONO or AS_CHANNEL_STEREO */
  );

  /**
   * @brief Initialize the MediaPlayer (Abridged version).
   *
   * @details This is abridged version on initialize API.
   *          If you would like to set DSP binary path but bit length paramter can be default,
   *          you can call this API. Bit length parameter will be defautl as below.
   *
   *          DSP binary path : /mnt/sd0/BIN
   *
   */

  err_t init(
      PlayerId id,            /**< Select Player ID. */
      uint8_t codec_type,     /**< Set compression code. AS_CODECTYPE_MP3 or AS_CODECTYPE_WAV */
      const char *codec_path, /**< Set DSP binary placement path */
      uint32_t sampling_rate, /**< Set sampling rate. AS_SAMPLINGRATE_XXXXX */
      uint8_t channel_number  /**< Set channnel number. AS_CHANNEL_MONO or AS_CHANNEL_STEREO */
  );

  /**
   * @brief Initialize the MediaPlayer.
   *
   * @details This is full version on initialize API.
   *          You can set all of decode parameters.
   *          Before you start playing, you must initialize media player by this API or
   *          abridge versions of this APS.
   *
   */

  err_t init(
      PlayerId id,            /**< Select Player ID. */
      uint8_t codec_type,     /**< Set compression code. AS_CODECTYPE_MP3 or AS_CODECTYPE_WAV */
      const char *codec_path, /**< Set DSP binary placement path */
      uint32_t sampling_rate, /**< Set sampling rate. AS_SAMPLINGRATE_XXXXX */
      uint8_t bit_length,     /**< Set bit length. AS_BITLENGTH_16 or AS_BITLENGTH_24 */
      uint8_t channel_number  /**< Set channnel number. AS_CHANNEL_MONO or AS_CHANNEL_STEREO */
  );

  /**
   * @brief Start playing.
   *
   * @details This function starts playing audio data.
   *          When you call this API, player system pull out audio data from FIFO.
   *          So, you have to push audio data which will play into FIFO by "write frame API".
   *          Decode completion and decodec data will be notified by callback function which is
   *          specified by this API. You can process them.
   *
   */

  err_t start(
      PlayerId id,            /**< Select Player ID. */
      DecodeDoneCallback dccb /**< Callback function for notify decode completion */
  );

  /**
   * @brief Stop playing.
   *
   * @details This function stops playing audio data.
   *          You can call this API when playing is activate.
   *          When you call this API, return of API will back soon but internal work
   *          cannot stop immediately. So, after API call, decodec data will be returned
   *          to application while supplied data runs out. It is better to process them.
   *
   */

  err_t stop(PlayerId id);

  /**
   * @brief Stop playing (Stop mode specify).
   *
   * @details This function stops playing audio data with stop mode.
   *          Stop mode is "Wait ES ends : stop after play all of supplied data and reply result"
   *          and "Immediately stop : stop soon". You can set either of them.
   *
   */

  err_t stop(
      PlayerId id, /**< Select Player ID. */
      uint8_t mode /**< Set stop mode. AS_STOPPLAYER_NORMAL or AS_STOPPLAYER_ESEND */
  );

  /**
   * @brief Request next process to player.
   *
   * @details This function requests next process to player.
   *          You should call this API at timing of next decode can be done.
   *          For example, timing of decoded data rendering is done.
   *
   */

  err_t reqNextProcess(
      PlayerId id,           /**< Select Player ID. */
      AsRequestNextType type /**< Set Request type. AsNextNormalRequest or AsNextStopResRequest */
  );

  /**
   * @brief Deactivate the MediaPlayer
   *
   * @details This function deactivates media player system.
   *
   */

  err_t deactivate(
      PlayerId id
  );

  /**
   * @brief Write(Supply) audio data to play
   *
   * @details This function supplies audio data to player system.
   *          When you call this API, player read "myfile" and supply audio data to FIFO
   *          and the data will be decoded. If you do not, FIFO will occuers underflow.
   *
   */

  err_t writeFrames(
      PlayerId id, /**< Select Player ID. */
      File& myfile /**< */
  );


private:

  /**
   * To avoid create multiple instance
   */

  MediaPlayer() {}
  MediaPlayer(const MediaPlayer&);
  MediaPlayer& operator=(const MediaPlayer&);
  ~MediaPlayer() {}


  CMN_SimpleFifoHandle m_player0_simple_fifo_handle;
  CMN_SimpleFifoHandle m_player1_simple_fifo_handle;

  uint32_t *m_player0_simple_fifo_buf;
  uint32_t *m_player1_simple_fifo_buf;

  AsPlayerInputDeviceHdlrForRAM m_player0_input_device_handler;
  AsPlayerInputDeviceHdlrForRAM m_player1_input_device_handler;

  char m_es_player0_buf[MEDIAPLAYER_BUF_FRAME_SIZE];
  char m_es_player1_buf[MEDIAPLAYER_BUF_FRAME_SIZE];

  err_t write_fifo(File& myFile, char *p_es_buf, CMN_SimpleFifoHandle *handle);
  bool check_decode_dsp(uint8_t codec_type, const char *path);
};

// #endif // __cplusplus
#endif // MediaPlayer_h

