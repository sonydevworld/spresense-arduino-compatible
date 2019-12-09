/*
 *  MediaRecorder.h - Audio include file for the Spresense SDK
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

/**
 * @file MediaRecorder.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Media Recorder Class for Arduino on Spresense.
 * @details By using this library, you can use the follow features on SPRESSENSE.
 *          - Voice recording
 */

#ifndef MediaRecorder_h
#define MediaRecorder_h

#ifdef SUBCORE
#error "Audio library is NOT supported by SubCore."
#endif

class File;
#include <audio/audio_high_level_api.h>
#include <audio/audio_message_types.h>
#include <audio/utilities/frame_samples.h>
#include <audio/utilities/wav_containerformat.h>
#include <memutils/simple_fifo/CMN_SimpleFifo.h>

#include "FrontEnd.h"

/*--------------------------------------------------------------------------*/

/**
 * MediaRecorder log output definition
 */

#define MEDIARECORDER_LOG_DEBUG

#define print_err printf

#ifdef MEDIARECORDER_LOG_DEBUG
#define print_dbg(...) printf(__VA_ARGS__)
#else
#define print_dbg(x...)
#endif

/**
 * MediaRecorder Error Code Definitions.
 */

#define MEDIARECORDER_ECODE_OK            0
#define MEDIARECORDER_ECODE_COMMAND_ERROR 1
#define MEDIARECORDER_ECODE_BUFFER_INIT_ERROR 2
#define MEDIARECORDER_ECODE_BUFFER_POLL_ERROR 3
#define MEDIARECORDER_ECODE_DSP_ACCESS_ERROR 4
#define MEDIARECORDER_ECODE_FILEACCESS_ERROR 5
#define MEDIARECORDER_ECODE_BUFFER_SIZE_ERROR 6
#define MEDIARECORDER_ECODE_BUFFER_AREA_ERROR 7
#define MEDIARECORDER_ECODE_INSUFFICIENT_BUFFER_AREA 8
#define MEDIARECORDER_ECODE_BASEBAND_ERROR 9
#define MEDIARECORDER_ECODE_BUFFER_ALLOC_ERROR 10

/**
 * MediaRecorder buffer size definition.
 */

#define MEDIARECORDER_BUF_FRAME_NUM 10
#define MEDIARECORDER_BUF_FRAME_SIZE (768 * 2 * 8)
#define MEDIARECORDER_BUF_SIZE (MEDIARECORDER_BUF_FRAME_NUM * MEDIARECORDER_BUF_FRAME_SIZE)

/**
 * MediaRecorder capturing clock mode.
 */

#define MEDIARECORDER_CAPCLK_NORMAL (0)
#define MEDIARECORDER_CAPCLK_HIRESO (1)

/*--------------------------------------------------------------------------*/

/**
 * @class MediaRecorder
 * @brief MediaRecorder Class Definitions.
 */

class MediaRecorder
{
public:

  /**
   * @brief Get instance of MediaRecorder for singleton.
   */

  static MediaRecorder* getInstance()
    {
      static MediaRecorder instance;
      return &instance;
    }

  /**
   * @brief Initialize the MediaRecorder.
   *
   * @details This function is called only once when using the MediaRecorder.
   *          In this function, alloc memory area of FIFO for ES data exchange,
   *          creation of objects for audio data capturing and encoding.
   *
   */

  err_t begin(void);

  /**
   * @brief Initialize the MediaRecorder.
   *
   * @details This function can set callback funtion which receive attention notify.
   *
   */

  err_t begin(AudioAttentionCb attcb);
  err_t begin(AudioAttentionCb attcb, bool use_frontend);

  /**
   * @brief Finalize the MediaRecorder.
   *
   * @details This function is called only once when finish the MediaRecorder.
   *
   */

  err_t end(void);

  /**
   * @brief Activate the MediaRecorder.
   *
   * @details This function activates media recorder system and audio HW.
   *          You should specify input device which you would like to record.
   *          You can set "Mic-in" or "I2S-in".
   *          The result of APIs will be returnd by callback function which is
   *          specified by this function.
   *
   */

  err_t activate(
      AsSetRecorderStsInputDevice input_device, /**< Select input device. AS_SETRECDR_STS_INPUTDEVICE_MIC or AS_SETRECDR_STS_INPUTDEVICE_I2S*/
      MediaRecorderCallback mrcb                /**< Sepcify callback function which is called to notify API results. */
  );

  /**
   * @brief Activate the MediaRecorder.
   *
   * @details This function works as same as above activate(input_device, mrcb).
   *          But is able to set buffer size of recorder.
   *
   */

  err_t activate(
      AsSetRecorderStsInputDevice input_device, /**< Select input device. AS_SETRECDR_STS_INPUTDEVICE_MIC or AS_SETRECDR_STS_INPUTDEVICE_I2S*/
      MediaRecorderCallback mrcb,               /**< Sepcify callback function which is called to notify API results. */
      uint32_t recorder_bufsize
  );


  /**
   * @brief Activate the MediaRecorder.
   *
   * @details This function works as same as above activate(input_device, mrcb, bufsize).
   *          But is able to set PreProcessing type. If ommit it, fix to Through.
   *
   */

  err_t activate(
      AsSetRecorderStsInputDevice input_device, /**< Select input device. AS_SETRECDR_STS_INPUTDEVICE_MIC or AS_SETRECDR_STS_INPUTDEVICE_I2S*/
      MediaRecorderCallback mrcb,               /**< Sepcify callback function which is called to notify API results. */
      uint32_t recorder_bufsize,
      AsMicFrontendPreProcType proc_type,
      bool is_digital = false
  );

  /**
   * @brief Initialize the MediaRecorder (Abridged version).
   *
   * @details This is abridged version on initialize API.
   *          You can init media recorder by only 2 parameters, Codec type and number of channels.
   *          When this API is called, other parameters are fixed as below.
   *
   *          Sampling Rate   : 48kHz
   *          Bit length      : 16bit
   *          Bit rate        : 96kbps(effective only when mp3)
   *          DSP binary path : /mnt/sd0/BIN
   *
   */

  err_t init(
      uint8_t codec_type,    /**< Select compression code. AS_CODECTYPE_MP3 or AS_CODECTYPE_WAV */
      uint8_t channel_number /**< Set chennel number. AS_CHANNEL_MONO or AS_CHANNEL_STEREO, 2CH, 4CH, 8CH */
  );

  /**
   * @brief Initialize the MediaRecorder (Abridged version).
   *
   * @details This is abridged version on initialize API.
   *          In this API, you can set initialize parameters exclude DSP binary path.
   *          If you don't heve needs to set DSP binary path, call this API.
   *          DSP binary path is fixed as below.
   *
   *          DSP binary path : /mnt/sd0/BIN
   *
   */

  err_t init(
      uint8_t codec_type,     /**< Select compression code. AS_CODECTYPE_MP3 or AS_CODECTYPE_WAV */
      uint8_t channel_number, /**< Set chennel number. AS_CHANNEL_MONO or AS_CHANNEL_STEREO, 2CH, 4CH, 8CH */
      uint32_t sampling_rate, /**< Set sampling rate. AS_SAMPLINGRATE_XXXXX */
      uint8_t bit_length,     /**< Set bit length. AS_BITLENGTH_16 or AS_BITLENGTH_24 */
      uint32_t bit_rate       /**< Set bit rate. AS_BITRATE_XXXXX */
  );

  /**
   * @brief Initialize the MediaRecorder
   *
   * @details This is full version on initialize API.
   *          In this API, you can should set all of initialize parameters.
   *          Before you start recording, you must initialize media recorder
   *          by this API or abridged versions of this API
   *
   */

  err_t init(
      uint8_t codec_type,     /**< Select compression code. AS_CODECTYPE_MP3 or AS_CODECTYPE_WAV */
      uint8_t channel_number, /**< Set chennel number. AS_CHANNEL_MONO or AS_CHANNEL_STEREO, 2CH, 4CH, 8CH */
      uint32_t sampling_rate, /**< Set sampling rate. AS_SAMPLINGRATE_XXXXX */
      uint8_t bit_length,     /**< Set bit length. AS_BITLENGTH_16 or AS_BITLENGTH_24 */
      uint32_t bit_rate,      /**< Set bit rate. AS_BITRATE_XXXXX */
      const char *codec_path  /**< Set DSP binary placement path */
  );

  /**
   * @brief Start Recording
   *
   * @details This function starts recoding.
   *          Once you call this function, the media recorder will be in active state
   *          and encoded data will be stored into internal FIFO. You shold pull out
   *          the data as soon as possible by "read frame API".
   *          If you do not, FIFO will overflow and data will lack.
   *
   *          This will continue until you call "stop API".
   *
   */

  err_t start(void);

  /**
   * @brief Stop Recording
   *
   * @details This function stops recoding.
   *          You can call this API when recording is activate.
   *          When you call this API, return of API will back soon but internal work
   *          cannot stop immediately. So, after API call, encoded data will be stored
   *          to FIFO a little. It is better to pull out them.
   *
   */

  err_t stop(void);

  /**
   * @brief Deactivate the MediaRecorder
   *
   * @details This function deactivates media recorder system, and audio HW.
   *
   */

  err_t deactivate(void);

  /**
   * @brief Set MicActivate the .
   *
   * @details The gain set when microphone is used can be specified
   *          by the "mic_gain" argument.
   *          You can set every 0.5 dB between 0 dB and 21 dB.
   *          In this parameter, a value from 0 to 210 is set for every 5.
   *
   */

  err_t setMicGain(int16_t mic_gain);

  /**
   * @brief Read a recorded audio data
   *
   * @details This function reads encoded audio data from media recorder.
   *          When you call this API, the data will be copied to p_buffer at buffer_size byte.
   *          Sometimes, a size of encoded data will be larger than buffer_size, but this API
   *          copies up to buffer_size. In the case, you should call this API again.
   *
   */

  err_t readFrames(
      uint8_t* p_buffer,
      uint32_t buffer_size,
      uint32_t* read_size
  );

  /**
   * @brief Write WAV header to file
   *
   * @details This function writes WAV header information to specified file.
   *          WAV heeader will be written top of the file.
   *
   */

  err_t writeWavHeader(File& myfile);

  /**
   * @brief Set capturing clock mode
   *
   * @details This API set internal audio capture clock mode to Normal(48kHz) or HiReso(192kHz).
   *          Default on boot is Normal, if you set HiReso, call this API with HiReso set.
   *
   */

  bool setCapturingClkMode(
      uint8_t clk_mode /**< Set clock mode. MEDIARECORDER_CAPCLK_NORMAL, MEDIARECORDER_CAPCLK_HIRESO */
  );

private:

  /**
   * To avoid create multiple instance
   */

  MediaRecorder()
    : m_recorder_simple_fifo_buf(NULL)
    , m_mr_callback(NULL)
    , m_p_fed_ins(NULL)
  {}
  MediaRecorder(const MediaRecorder&);
  MediaRecorder& operator=(const MediaRecorder&);
  ~MediaRecorder() {}

  /**
   * For audio data FIFO
   */

  CMN_SimpleFifoHandle m_recorder_simple_fifo_handle;
  uint32_t             *m_recorder_simple_fifo_buf;
  char                 m_es_recorder_buf[MEDIARECORDER_BUF_FRAME_SIZE];

  AsRecorderOutputDeviceHdlr m_output_device_handler;
  int                        m_es_size;
  WAVHEADER                  m_wav_format;
  MediaRecorderCallback      m_mr_callback;

  FrontEnd *m_p_fed_ins;

  bool check_encode_dsp(uint8_t codec_type, const char *path, uint32_t sampling_rate);

  /**
   * Initialize functions
   */

  void init_wav(AsInitRecorderParam *param);
  void init_mp3(AsInitRecorderParam *param);
  void init_opus(AsInitRecorderParam *param);
  void init_pcm(AsInitRecorderParam *param);
};

#endif // MediaRecorder_h

