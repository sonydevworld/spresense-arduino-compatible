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

#ifndef MediaRecorder_h
#define MediaRecorder_h

#include <SDHCI.h>
#include <audio/audio_high_level_api.h>
#include <memutils/simple_fifo/CMN_SimpleFifo.h>

#include "WavHeaderdef.h"

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
#define MEDIARECORDER_ECODE_BASEBAND_ERROR 8

/**
 * MediaRecorder buffer size definition.
 */

#define MEDIARECORDER_BUF_FRAME_NUM 10 
#define MEDIARECORDER_BUF_FRAME_SIZE (768 * 2 * 8) 
#define MEDIARECORDER_BUF_SIZE (MEDIARECORDER_BUF_FRAME_NUM * MEDIARECORDER_BUF_FRAME_SIZE)

/*--------------------------------------------------------------------------*/

/**
 * MediaRecorder Class Definitions.
 */

class MediaRecorder
{
public:

  err_t begin(void);
  err_t end(void);
  err_t activate(AsSetRecorderStsInputDevice input_device, MediaRecorderCallback mrcb);
  err_t init(uint8_t codec_type, uint8_t channel_number);
  err_t init(uint8_t codec_type, uint8_t channel_number, uint32_t sampling_rate, uint8_t bit_length, uint32_t bit_rate);
  err_t init(uint8_t codec_type, uint8_t channel_number, uint32_t sampling_rate, uint8_t bit_length, uint32_t bit_rate, const char *codec_path);
  err_t start(void);
  err_t stop(void);
  err_t deactivate(void);
  err_t readFrames(uint8_t* p_buffer, uint32_t buffer_size, uint32_t* read_size);
  err_t writeWavHeader(File& myfile);

  /**
   * To get instance of MediaRecorder 
   */

  static MediaRecorder* getInstance()
    {
      static MediaRecorder instance;
      return &instance;
    }

private:

  /**
   * To avoid create multiple instance
   */

  MediaRecorder() {}
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
  WavFormat_t               m_wav_format;

  bool check_encode_dsp(uint8_t codec_type, const char *path, uint32_t sampling_rate);

  /**
   * Initialize functions
   */

  void init_wav(AsInitRecorderParam *param);
  void init_mp3(AsInitRecorderParam *param);
  void init_opus(AsInitRecorderParam *param);
  void init_pcm(AsInitRecorderParam *param);

  /**
   * Baseband setting 
   */

  bool activateBaseband(void);
  bool deactivateBaseband(void);
};

#endif // MediaRecorder_h

