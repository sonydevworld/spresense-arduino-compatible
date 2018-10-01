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

#ifndef OutputMixer_h 
#define OutputMixer_h

// #ifdef __cplusplus

#include <audio/audio_high_level_api.h>

/*--------------------------------------------------------------------------*/

/**
 * MediaPlayer log output definition
 */
#define OUTPUTMIXER_LOG_DEBUG

#define print_err printf

#ifdef OUTPUTMIXER_LOG_DEBUG
#define print_dbg(...) printf(__VA_ARGS__)
#else
#define print_dbg(x...)
#endif

/**
 * MediaPlayer Error Code Definitions.
 */

#define OUTPUTMIXER_ECODE_OK            0
#define OUTPUTMIXER_ECODE_COMMAND_ERROR 1 

/*--------------------------------------------------------------------------*/

/**
 * @class OutputMixer
 * @brief OutputMixer Class Definitions.
 */

class OutputMixer
{
public:

  /**
   * @brief Get instance of MediaRecorder for singleton.
   */

  static OutputMixer* getInstance()
    {
      static OutputMixer instance;
      return &instance;
    }

  /**
   * @brief Creation of the OutputMixer.
   *
   * @details This function is called only once when using the OutputMixer.
   *          In this function, create objcets for audio data mixing and rendering. 
   *
   */

  err_t create(void);

  /**
   * @brief Activate the OutputMixer 
   *
   * @details This function activates output mixer system.
   *          The result of APIs will be returnd by callback function which is
   *          specified by this function.
   *
   */

  err_t activate(
      AsOutputMixerHandle handle, /**< Select output mixer handle. OutputMixer0 or OutputMixer1 */
      OutputMixerCallback omcb    /**< Sepcify callback function which is called to notify API results. */
  );

  /**
   * @brief Send PCM data via OutputMixer 
   *
   * @details This function send PCM data.
   *          According to "pcm" paramters, start sending PCM data.
   *          When send complete, callback function "pcmdome_cb" will be called.
   *
   */

  err_t sendData(
      AsOutputMixerHandle handle,     /**< Select output mixer handle. OutputMixer0 or OutputMixer1 */
      PcmProcDoneCallback pcmdone_cb, /**< Callback function which will be called when send complete */
      AsPcmDataParam pcm              /**< PCM data parameters */
  );

  /**
   * @brief Deactivate the OutputMixer 
   *
   * @details This function deactivates output mixer system.
   *
   */

  err_t deactivate(
      AsOutputMixerHandle handle      /**< Select output mixer handle. OutputMixer0 or OutputMixer1 */
  );

  /**
   * @brief Activate Audio Hw 
   *
   * @details This function activates Audio HW.
   *          You must call this API to sound.
   *
   */

  err_t activateBaseband(void);

  /**
   * @brief Deactivate Audio Hw 
   *
   * @details This function deactivates output mixer system.
   *
   */

  err_t deactivateBaseband(void);

  /**
   * @brief Set speaker out volume
   *
   * @details This function set volume.
   *          You should activate baseband before this API call.
   *
   */

  err_t setVolume(
      int master,  /**< Master volume. -1020(-102db) - 120(12db) */
      int player0, /**< Player0 volume. -1020(-102db) - 120(12db) */
      int player1  /**< Plyaer1 volume. -1020(-102db) - 120(12db) */
  );

private:

  /**
   * To avoid create multiple instance
   */

  OutputMixer() {}
  OutputMixer(const OutputMixer&);
  OutputMixer& operator=(const OutputMixer&);
  ~OutputMixer() {}

};

// #endif // __cplusplus
#endif // MediaPlayer_h

