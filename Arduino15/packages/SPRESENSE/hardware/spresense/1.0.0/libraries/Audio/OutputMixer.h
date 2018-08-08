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
 * OutputMixer Class Definitions.
 */

class OutputMixer
{
public:

  err_t create(void);
  err_t activate(AsOutputMixerHandle handle, OutputMixerCallback omcb);
  err_t sendData(AsOutputMixerHandle handle,
                 PcmProcDoneCallback pcmdone_cb,
                 AsPcmDataParam pcm);
  err_t deactivate(AsOutputMixerHandle handle);

  err_t activateBaseband(void);
  err_t deactivateBaseband(void);
  err_t setVolume(int master, int player0, int player1);

  /**
   * To get instance of OutputMixer 
   */

  static OutputMixer* getInstance()
    {
      static OutputMixer instance;
      return &instance;
    }

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

