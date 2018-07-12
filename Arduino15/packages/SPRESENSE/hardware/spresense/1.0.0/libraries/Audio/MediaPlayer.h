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

#ifdef __cplusplus

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

#define MEDIAPLAYER_BUF_FRAME_NUM  10 
#define MEDIAPLAYER_BUF_FRAME_SIZE 3072
#define MEDIAPLAYER_BUF_SIZE (MEDIAPLAYER_BUF_FRAME_NUM * MEDIAPLAYER_BUF_FRAME_SIZE)

/*--------------------------------------------------------------------------*/

/**
 * MediaPlayer Class Definitions.
 */

class MediaPlayer
{
public:

  typedef enum
  {
    Player0,
    Player1
  } PlayerId;

  err_t begin(void);
  err_t create(PlayerId id);
  err_t activate(PlayerId id, uint8_t output_device, MediaPlayerCallback mpcb);
  err_t init(PlayerId id, uint8_t codec_type, const char *codec_path, uint32_t sampling_rate, uint8_t channel_number);
  err_t start(PlayerId id, DecodeDoneCallback dccb);
  err_t stop(PlayerId id);
  err_t reqNextProcess(PlayerId id, AsRequestNextType type);
  err_t deactivate(PlayerId id);

  err_t writeFrames(
      PlayerId id, /**< Player ID の指定 */
      File& myfile /**< 音声ファイルを制御しているFileクラスのインスタンスを指定します。*/
  );

  /**
   * To get instance of MediaPlayer 
   */

  static MediaPlayer* getInstance()
    {
      static MediaPlayer instance;
      return &instance;
    }

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

#endif // __cplusplus
#endif // MediaPlayer_h

