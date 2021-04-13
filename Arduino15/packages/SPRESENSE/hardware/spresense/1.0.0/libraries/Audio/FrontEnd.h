/*
 *  FrontEnd.h - Audio include file for the Spresense SDK
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

/**
 * @file FrontEnd.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Mic Frontend Class for Arduino on Spresense.
 * @details By using this library, you can use the following features on Spresense.
 *          - Sound capture
 *          - Sound effector
 */

#ifndef FrontEnd_h
#define FrontEnd_h

#ifdef SUBCORE
#error "Audio library is NOT supported by SubCore."
#endif

#include <audio/audio_capture_api.h>
#include <audio/audio_frontend_api.h>
#include <audio/audio_message_types.h>
#include <audio/utilities/frame_samples.h>
#include <audio/utilities/wav_containerformat.h>
#include <memutils/simple_fifo/CMN_SimpleFifo.h>

/*--------------------------------------------------------------------------*/

/**
 * FrontEnd log output definition
 */

#define FRONTEND_LOG_DEBUG

#define print_err printf

#ifdef FRONTEND_LOG_DEBUG
#define print_dbg(...) printf(__VA_ARGS__)
#else
#define print_dbg(x...)
#endif

/**
 * FrontEnd Error Code Definitions.
 */

#define FRONTEND_ECODE_OK            0
#define FRONTEND_ECODE_COMMAND_ERROR 1
#define FRONTEND_ECODE_BASEBAND_ERROR 2 

/**
 * MediaRecorder capturing clock mode.
 */

#define FRONTEND_CAPCLK_NORMAL (0)
#define FRONTEND_CAPCLK_HIRESO (1)

/*--------------------------------------------------------------------------*/

/**
 * @class FrontEnd 
 * @brief FrontEnd Class Definitions.
 */

class FrontEnd
{
public:

  /**
   * @brief Get instance of FrontEnd for singleton.
   */

  static FrontEnd* getInstance()
    {
      static FrontEnd instance;
      return &instance;
    }

  /**
   * @brief Initialize the FrontEnd.
   *
   * @details This function is called only once when using the FrontEnd.
   *          In this function, creat objects for audio data capturing and filtering.
   *
   */

  err_t begin(void);

  /**
   * @brief Initialize the FrontEnd.
   *
   * @details This function can set callback funtion which receive attention notify.
   *
   */

  err_t begin(AudioAttentionCb attcb);

  /**
   * @brief Finalize the FrontEnd.
   *
   * @details This function is called only once when finish the MediaRecorder.
   *
   */

  err_t end(void);

  /**
   * @brief Activate the FrontEnd.
   *
   * @details This function activates frontend system and audio HW.
   *          You can specify pre-process type.
   *
   */

  err_t activate(void);

  /**
   * @brief Activate the FrontEnd.
   *
   * @details This function activates frontend system and audio HW.
   *          You can specify preproces type.
   *          If you activate FrontEnd by this API, The result of all APIs
   *          will be returnd by callback function which is specified by this function.
   *          (It means that the return of API is not completion of internal process)
   *
   */

  err_t activate(
      MicFrontendCallback fedcb
  );
  err_t activate(
      MicFrontendCallback fedcb,
      bool                is_digital
  );

  /**
   * @brief Initialize the FrontEnd 
   *
   * @details This is full version on initialize API.
   *          In this API, you can should set all of initialize parameters.
   *          Before you start FrontEnd, you must initialize by this API.
   *
   */

  err_t init(
      uint8_t channel_number,     /**< Set chennel number. AS_CHANNEL_MONO or AS_CHANNEL_STEREO, 2CH, 4CH, 8CH */
      uint8_t bit_length,         /**< Set bit length. AS_BITLENGTH_16 or AS_BITLENGTH_24 */
      uint32_t samples_per_frame, /**< Number of Samples per one frame */
      uint8_t data_path,          /**< Set capture data path. AsDataPathCallback or AsDataPathMessage */ 
      AsDataDest dest             /**< Destination of outgoing data from FrontEnd */
  );

  /**
   * @brief Start FrontEnd 
   *
   * @details This function starts FrontEnd.
   *          Once you call this function, the FrontEnc will be in active state
   *          and start capturing data. Captured data will be pre-processed and
   *          deliveryed to data destination which is set by init() API.
   *          This will continue until you call "stop API".
   *
   */

  err_t start(void);

  /**
   * @brief Stop Recording
   *
   * @details This function stops FrontEnd.
   *          You can call this API when FrontEnd is activate.
   *
   */

  err_t stop(void);

  /**
   * @brief Send init command to pre-process DSP. 
   *
   * @details This function sends Init command to pre-process DSP.
   *
   */

  err_t initpreproc(
      AsInitPreProcParam *param /**< Init command packet parameter for pre-process DSP */
  );

  /**
   * @brief Send init command to pre-process DSP. 
   *
   * @details This function sends Set command to pre-process DSP.
   *
   */

  err_t setpreproc(
      AsSetPreProcParam *param /**< Set command packet parameter for pre-process DSP */
  );

  /**
   * @brief Set Mic gain.
   *
   * @details The gain set when microphone is used can be specified
   *          by the "mic_gain" argument.
   *          You can set every 0.5 dB between 0 dB and 21 dB.
   *          In this parameter, a value from 0 to 210 is set for every 5.
   *
   */

  err_t setMicGain(
      int16_t mic_gain /**< Input gain : value range
                          Analog Mic  0:0dB, 5:+0.5dB, ... , 210:+21.0dB (Max is 21dB.)
                          Digital Mic -7850:-78.50dB, ... , -5:-0.05dB, 0:0dB (Max is 0dB.)
                          set #AS_MICGAIN_HOLD is keep setting. */
  );

  /**
   * @brief Set capturing clock mode
   *
   * @details This API set internal audio capture clock mode to Normal(48kHz) or HiReso(192kHz).
   *          Default on boot is Normal, if you set HiReso, call this API with HiReso set.
   *
   */

  err_t setCapturingClkMode(
      uint8_t clk_mode /**< Set clock mode. FRONTEND_CAPCLK_NORMAL, FRONTEND_CAPCLK_HIRESO */
  );

  /**
   * @brief Deactivate the FrontEnd 
   *
   * @details This function deactivates FrontEnd system, and audio HW.
   *
   */

  err_t deactivate(void);

private:

  /**
   * To avoid create multiple instance
   */

  FrontEnd()
    : m_fed_callback(NULL)
  {}
  FrontEnd(const FrontEnd&);
  FrontEnd& operator=(const FrontEnd&);
  ~FrontEnd() {}

  /**
   * Private members
   */

  MicFrontendCallback m_fed_callback;

  /**
   * Baseband setting
   */

  err_t activateBaseband(void);
  err_t deactivateBaseband(void);

};

#endif // FrontEnd_h

