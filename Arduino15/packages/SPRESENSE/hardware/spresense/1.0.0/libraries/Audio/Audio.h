/*
 *  Audio.h - Audio include file for the Spresense SDK
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
 * @file Audio.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Audio Library Class for Arduino on Spresense.
 * @details By using this library, you can use the follow features on SPRESSENSE.
 *          - Music playback
 *          - Voice recording
 */

#ifndef Audio_h
#define Audio_h

#ifdef SUBCORE
#error "Audio library is NOT supported by SubCore."
#endif

/**
 * @defgroup audio Audio Library API
 * @brief API for using Audio
 * @{
 */

#include <pins_arduino.h>

class File;

// #ifdef __cplusplus

#include <audio/audio_high_level_api.h>
#include <audio/utilities/wav_containerformat.h>
#include <audio/utilities/wav_containerformat_parser.h>
#include <audio/utilities/frame_samples.h>
#include <memutils/simple_fifo/CMN_SimpleFifo.h>

#define WRITE_FIFO_FRAME_NUM  (8)
#define WRITE_FIFO_FRAME_SIZE (1024*2*3)
#define WRITE_BUF_SIZE   (WRITE_FIFO_FRAME_NUM * WRITE_FIFO_FRAME_SIZE)

#define READ_FIFO_FRAME_NUM   (10)
#define READ_FIFO_FRAME_SIZE  (1024*2*8)
#define READ_BUF_SIZE    (READ_FIFO_FRAME_NUM * READ_FIFO_FRAME_SIZE)

#define FIFO_FRAME_SIZE (\
                          (WRITE_BUF_SIZE > READ_BUF_SIZE) ?\
                          (WRITE_FIFO_FRAME_SIZE) : (READ_FIFO_FRAME_SIZE)\
                        )

#define SIMPLE_FIFO_BUF_SIZE (\
                               ((WRITE_BUF_SIZE) > (READ_BUF_SIZE)) ? \
                               (WRITE_BUF_SIZE) : (READ_BUF_SIZE)\
                             )

extern "C" void  outputDeviceCallback(uint32_t);

/*--------------------------------------------------------------------------*/
#define print_err printf

#ifdef BRD_DEBUG
#define print_dbg(...) printf(__VA_ARGS__)
#else
#define print_dbg(x...)
#endif

/*--------------------------------------------------------------------------*/

#define AS_CODECTYPE_PCM  5

/*--------------------------------------------------------------------------*/
/**
 * Audio Library Error Code Definitions.
 */
#define AUDIOLIB_ECODE_OK                  0  /**< Execution result OK */
#define AUDIOLIB_ECODE_SHARED_MEMORY_ERROR 1  /**< */
#define AUDIOLIB_ECODE_SIMPLEFIFO_ERROR    2  /**< */
#define AUDIOLIB_ECODE_AUDIOCOMMAND_ERROR  3  /**< */
#define AUDIOLIB_ECODE_FILEACCESS_ERROR    4  /**< */
#define AUDIOLIB_ECODE_FILEEND             5  /**< */
#define AUDIOLIB_ECODE_BUFFER_AREA_ERROR   6  /**< */
#define AUDIOLIB_ECODE_BUFFER_SIZE_ERROR   7  /**< */
#define AUDIOLIB_ECODE_INSUFFICIENT_BUFFER_AREA   8  /**< */
#define AUDIOLIB_ECODE_WAV_PARSE_ERROR     9  /**< */
#define AUDIOLIB_ECODE_PARAMETER_ERROR    10  /**< */

/*--------------------------------------------------------------------------*/
/**
 * Audio Library Type Definitions.
 */

typedef unsigned int err_t;

/*--------------------------------------------------------------------------*/

/**
 * @class AudioClass
 * @brief Audio Library Class Definitions.
 */
class AudioClass
{
public:

  /**
   * @brief Get instance of AudioClass for singleton.
   */
  static AudioClass* getInstance()
    {
      static AudioClass instance;
      return &instance;
    }

  /**
   * @enum PlayerId
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
   * @brief Initialize the audio library and HW modules.
   *
   * @details This function is called only once when using the Audio library.
   *          In this function, initialization of required shared memory management library,
   *          initialization of inter-task communication library, Initialize Audio MW,
   *          initialize FIFO for ES supply, set callback at error occurrence, etc.
   *
   *          If you call it more than once, an error occurs,
   *          but if you call "end ()" you need to call this function again.
   *
   */
  err_t begin(void);

  /**
   * @brief Initialize the audio library and HW modules.
   *
   * @details This function can set callback funtion which receive attention notify.
   *
   */

  err_t begin(AudioAttentionCb attcb);

  /**
   * @brief Finalization the audio library and HW modules.
   *
   * @details This function is called when you want to exit the Audio library.
   *          In this function, necessary termination processing of the shared memory management
   *          library, end processing of the inter-task communication library,
   *          end processing of Audio MW, destruction of FIFO for ES supply,
   *          clearing of callback setting at error occurrence, etc.
   *
   *          This can only be called once when activated.
   *          If you call it before calling "begin ()" or call it more than once, an error occurs.
   *
   */
  err_t end(void);

  /**
   * @brief Set Audio Library Mode to Music Player.
   *
   * @details This function switches the mode of the Audio library to Music Player.
   *          For mode details, follow the state transition chart on the developer guide.
   *
   *          This function cannot be called after transition to "Music Player mode".
   *          To return to the original state, please call setReadyMode ().
   *
   *          In this function, setting HW necessary for music playback, and setting ES buffer configuration etc.
   *
   */
  err_t setPlayerMode(
      uint8_t device  /**< Select output device. AS_SETPLAYER_OUTPUTDEVICE_SPHP or
                           AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT. */
  );

  /**
   * @brief Set Audio Library Mode to Music Player.
   *
   * @details This API has same function as setPlayerMode(device).
   *          But you can set buffer size of players.
   *
   */
  err_t setPlayerMode(
      uint8_t device,          /**< Select output device. AS_SETPLAYER_OUTPUTDEVICE_SPHP or
                                    AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT. */
      uint32_t player0bufsize, /**< Buffer size of player 0. Must be n > 0. */
      uint32_t player1bufsize  /**< Buffer size of player 1. Must be n > 0. */
  );

  /**
   * @brief Set Audio Library Mode to Music Player.
   *
   * @details This function works as same as "setPlayerMode(uint8_t)",
   *          but you are able to set speaker drive mode by parameter "sp_drv".
   *          If you would like to set speaker drive mode, use this API instead of setPlayerMode(uint8_t).
   *
   */
  err_t setPlayerMode(
      uint8_t device, /**< Select output device. AS_SETPLAYER_OUTPUTDEVICE_SPHP or 
                           AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT. */
      uint8_t sp_drv /**< Select audio speaker driver mode. AS_SP_DRV_MODE_LINEOUT or
                          AS_SP_DRV_MODE_1DRIVER or AS_SP_DRV_MODE_2DRIVER or AS_SP_DRV_MODE_4DRIVER */
  );

  /**
   * @brief Set Audio Library Mode to Music Player.
   *
   * @details This API has same function as setPlayerMode(device, sp_drv).
   *          But you can set buffer size of players.
   *
   */
  err_t setPlayerMode(
      uint8_t device,          /**< Select output device. AS_SETPLAYER_OUTPUTDEVICE_SPHP or 
                                    AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT. */
      uint8_t sp_drv,          /**< Select audio speaker driver mode. AS_SP_DRV_MODE_LINEOUT or
                                    AS_SP_DRV_MODE_1DRIVER or AS_SP_DRV_MODE_2DRIVER or AS_SP_DRV_MODE_4DRIVER */
      uint32_t player0bufsize, /**< Buffer size of player 0. Must be n > 0. */
      uint32_t player1bufsize  /**< Buffer size of player 1. Must be n > 0. */
  );

  /**
   * @brief Set Audio Library Mode to Sound Recorder.
   *
   * @details This function switches the mode of the Audio library to Sound Recorder.
   *          For mode details, follow the state transition chart on the developer guide.
   *
   *          This function cannot be called after transition to "Sound Recorder mode".
   *          To return to the original state, please call setReadyMode ().
   *
   *          In this function, setting HW necessary for sound recording, and setting ES buffer configuration etc.
   *
   *          The type of MIC is determined by configuration of sdk. The default is an analog microphone.
   *          The value of CXD56_AUDIO_MIC_CHANNEL_SEL can be used to select the analog microphone
   *          or digital microphone and assign the microphone channel.
   *          Refer to the document for the setting value.
   *          If you change the settings, import sdk again.
   *
   *          Recording Mic-gain is 0dB fixed.
   *
   */
  err_t setRecorderMode(
      uint8_t input_device /**< Set AS_SETRECDR_STS_INPUTDEVICE_MIC. */
  );

  /**
   * @brief Set Audio Library Mode to Sound Recorder.
   *
   * @details This function works as same as "setRecorderMode(uint8_t)",
   *          but you are able to set Mic-gain by parameter "mic_gain".
   *          If you would like to set recording mic-gain, use this API instead of  setRecorderMode(uint8_t).
   *
   */
  err_t setRecorderMode(
      uint8_t input_device, /**< Set AS_SETRECDR_STS_INPUTDEVICE_MIC. */
      int32_t input_gain    /**< Input gain : value range
                                 Analog Mic  0:0dB, 5:+0.5dB, ... , 210:+21.0dB (Max is 21dB.)
                                 Digital Mic -7850:-78.50dB, ... , -5:-0.05dB, 0:0dB (Max is 0dB.)
                                 set #AS_MICGAIN_HOLD is keep setting. */
  );

  /**
   * @brief Set Audio Library Mode to Sound Recorder.
   *
   * @details This function works as same as "setRecorderMode(input_device, input_gain)",
   *          But you can set buffer size of recorder.
   *
   */
  err_t setRecorderMode(
      uint8_t input_device, /**< Set AS_SETRECDR_STS_INPUTDEVICE_MIC. */
      int32_t input_gain,   /**< Input gain : value range
                                 Analog Mic  0:0dB, 5:+0.5dB, ... , 210:+21.0dB (Max is 21dB.)
                                 Digital Mic -7850:-78.50dB, ... , -5:-0.05dB, 0:0dB (Max is 0dB.)
                                 set #AS_MICGAIN_HOLD is keep setting. */
      uint32_t bufsize      /**< Buffer size of recorder. */
  );

  /**
   * @brief Set Audio Library Mode to Sound Recorder.
   *
   * @details This function works as same as "setRecorderMode(input_device, input_gain, bufsize)",
   *          But you can set buffer size of recorder.
   *
   */
  err_t setRecorderMode(
      uint8_t input_device, /**< Set AS_SETRECDR_STS_INPUTDEVICE_MIC. */
      int32_t input_gain,   /**< Input gain : value range
                                 Analog Mic  0:0dB, 5:+0.5dB, ... , 210:+21.0dB (Max is 21dB.)
                                 Digital Mic -7850:-78.50dB, ... , -5:-0.05dB, 0:0dB (Max is 0dB.)
                                 set #AS_MICGAIN_HOLD is keep setting. */
      uint32_t bufsize,     /**< Buffer size of recorder. */
      bool is_digital       /**< Select mic type. true:Digital, false:Analog. */
  );

  /**
   * @enum ThroughInput
   *
   * @brief Input select parameter at baseband through mode.
   */

  typedef enum
  {
    MicIn,  /**< Use Microphone only. */
    I2sIn,  /**< Use I2S input only. */
    BothIn  /**< Use Microphone and I2S input. */
  } ThroughInput;

  /**
   * @enum ThroughI2sOut
   *
   * @brief I2S output select parameter at baseband through mode.
   */

  typedef enum
  {
    None,   /**< I2S Output is nothing. */
    Mixer,  /**< I2S Output is Mixer output data. */
    Mic     /**< I2S Output is Microphone. */
  } ThroughI2sOut;

  /**
   * @brief Set Audio Library Mode to BaseBand Through.
   *
   * @details This function switches the mode of the Audio library to the Baseband through state.
   *  (For low power, low latency, small memory)
   *
   *          For mode details, follow the state transition chart on the developer guide.
   *
   *          You can chose input and I2S output data, but speaker output is always mixer data.
   *          And, in this mode, output starts as soon as the state changes.
   *
   *          This function cannot be called after transition to "BaseBand Through mode".
   *          To return to the original state, please call setReadyMode ().
   *
   */
  err_t setThroughMode(
      ThroughInput input,     /**< Set input source. Use ThroughInput enum type. */
      ThroughI2sOut i2s_out,  /**< Set I2S output source. Use ThroughI2sOut enum type. */
      bool sp_out,            /**< sp_out : Use speaker output. */
      int32_t input_gain,     /**< Input gain : value range
                                 Analog Mic  0:0dB, 5:+0.5dB, ... , 210:+21.0dB (Max is 21dB.)
                                 Digital Mic -7850:-78.50dB, ... , -5:-0.05dB, 0:0dB (Max is 0dB.)
                                 set #AS_MICGAIN_HOLD is keep setting. */
      uint8_t sp_drv /**< Select audio speaker driver mode. AS_SP_DRV_MODE_LINEOUT or
                          AS_SP_DRV_MODE_1DRIVER or AS_SP_DRV_MODE_2DRIVER or AS_SP_DRV_MODE_4DRIVER */
  );

  /**
   * @brief Set Audio Library Mode to Ready.
   *
   * @details This function switches the mode of the Audio library to the initial state.
   *          For mode details, follow the state transition chart on the developer guide.
   *
   *          This function cannot be called after transition to "Ready mode".
   *          Immediately after boot, it is in Ready mode.
   *
   *          In this function, we will release resources which used in each mode, change HW to the standby state, etc.
   *
   */
  err_t setReadyMode(void);

  /**
   * @brief Initialize player.
   *
   * @details This function initializes and sets Player action.
   *          When player do not play music, you can call it as many times as you like.
   *
   *          By this function,
   *            - Compression codec
   *            - Sampling rate
   *            - Number of channels
   *          You need to set.
   *
   *          If you would like to set codec binary path and bit length of audio data,
   *          You are able to set them by other type of this API.
   *          Default values of them are /mnt/sd0/BIN and AS_BITLENGTH_16(16bit).
   *
   */
  err_t initPlayer(
      PlayerId id,    /**< Select Player ID. */
      uint8_t codec,  /**< Select compression code. AS_CODECTYPE_MP3 or AS_CODECTYPE_WAV */
      uint32_t fs,    /**< Set sampling rate. AS_SAMPLINGRATE_XXXXX. */
      uint8_t channel /**< Set channnel number. AS_CHANNEL_MONO or AS_CHANNEL_STEREO */
  );

  /**
   * @brief Initialize player.
   *
   * @details This function initializes and sets Player action.
   *          When player do not play music, you can call it as many times as you like.
   *
   *          By this function,
   *            - Compression codec
   *            - Sampling rate
   *            - Number of channels
   *          You need to set.
   *
   *          If you would like to set codec binary path and bit length of audio data,
   *          You are able to set them by other type of this API.
   *          Default values of them are /mnt/sd0/BIN and AS_BITLENGTH_16(16bit).
   *
   */
  err_t initPlayer(
      PlayerId id,    /**< Select Player ID. */
      uint8_t codec,  /**< Select compression code. AS_CODECTYPE_MP3 or AS_CODECTYPE_WAV */
      uint32_t fs,    /**< Set sampling rate. AS_SAMPLINGRATE_XXXXX. */
      uint8_t bitlen, /**< Set bit length. AS_BITLENGTH_16 or AS_BITLENGTH_24 */
      uint8_t channel /**< Set channnel number. AS_CHANNEL_MONO or AS_CHANNEL_STEREO */
  );

  /**
   * @brief Initialize player.
   *
   * @details This function initializes and sets Player action.
   *          When player do not play music, you can call it as many times as you like.
   *
   *          By this function,
   *            - Compression codec
   *            - Sampling rate
   *            - Number of channels
   *          You need to set.
   *
   *          If you would like to set codec binary path and bit length of audio data,
   *          You are able to set them by other type of this API.
   *          Default values of them are /mnt/sd0/BIN and AS_BITLENGTH_16(16bit).
   *
   */
  err_t initPlayer(
      PlayerId id,    /**< Select Player ID. */
      uint8_t codec,  /**< Select compression code. AS_CODECTYPE_MP3 or AS_CODECTYPE_WAV */
      const char *codec_path,  /**< Set DSP Binary path. Maximum length is 24 bytes.*/
      uint32_t fs,    /**< Set sampling rate. AS_SAMPLINGRATE_XXXXX. */
      uint8_t channel /**< Set channnel number. AS_CHANNEL_MONO or AS_CHANNEL_STEREO */
  );

  /**
   * @brief Initialize player.
   *
   * @details This function initializes and sets Player action.
   *          When player do not play music, you can call it as many times as you like.
   *
   *          By this function,
   *            - Compression codec
   *            - Sampling rate
   *            - Number of channels
   *          You need to set.
   *
   *          If you would like to set codec binary path and bit length of audio data,
   *          You are able to set them by other type of this API.
   *          Default values of them are /mnt/sd0/BIN and AS_BITLENGTH_16(16bit).
   *
   */
  err_t initPlayer(
      PlayerId id,    /**< Select Player ID. */
      uint8_t codec,  /**< Select compression code. AS_CODECTYPE_MP3 or AS_CODECTYPE_WAV */
      const char *codec_path,  /**< Set DSP Binary path. Maximum length is 24 bytes.*/
      uint32_t fs,    /**< Set sampling rate. AS_SAMPLINGRATE_XXXXX. */
      uint8_t bitlen, /**< Set bit length. AS_BITLENGTH_16 or AS_BITLENGTH_24 */
      uint8_t channel /**< Set channnel number. AS_CHANNEL_MONO or AS_CHANNEL_STEREO */
  );

  /**
   * @brief Initialize recorder
   *
   * @details This function initializes and sets Recorder action.
   *          When recorder do not start, you can call it as many times as you like.
   *
   *          By this function,
   *            - Compression codec
   *            - Sampling rate
   *            - Number of channels
   *          You need to set.
   *
   *          If you would like to set codec binary path and bit length of audio data,
   *          You are able to set them by other type of this API.
   *          Default values of them are /mnt/sd0/BIN and AS_BITLENGTH_16(16bit).
   *
   */
  err_t initRecorder(
      uint8_t codec,  /**< Select compression code. AS_CODECTYPE_MP3 or AS_CODECTYPE_WAV */
      uint32_t fs,    /**<Set sampling rate. AS_SAMPLINGRATE_XXXXX. */
      uint8_t channel /**< Set channnel number. AS_CHANNEL_MONO, AS_CHANNEL_STEREO, AS_CHANNEL_4CH, or etc...  */
  );

  /**
   * @brief Initialize recorder
   *
   * @details This function initializes and sets Recorder action.
   *          When recorder do not start, you can call it as many times as you like.
   *
   *          By this function,
   *            - Compression codec
   *            - Sampling rate
   *            - Number of channels
   *          You need to set.
   *
   *          If you would like to set codec binary path and bit length of audio data,
   *          You are able to set them by other type of this API.
   *          Default values of them are /mnt/sd0/BIN and AS_BITLENGTH_16(16bit).
   *
   */
  err_t initRecorder(
      uint8_t codec,  /**< Select compression code. AS_CODECTYPE_MP3 or AS_CODECTYPE_WAV */
      uint32_t fs,    /**<Set sampling rate. AS_SAMPLINGRATE_XXXXX. */
      uint8_t bitlen, /**< Set bit length. AS_BITLENGTH_16 or AS_BITLENGTH_24 */
      uint8_t channel /**< Set channnel number. AS_CHANNEL_MONO, AS_CHANNEL_STEREO, AS_CHANNEL_4CH, or etc...  */
  );

  /**
   * @brief Initialize recorder
   *
   * @details This function initializes and sets Recorder action.
   *          When recorder do not start, you can call it as many times as you like.
   *
   *          By this function,
   *            - Compression codec
   *            - Sampling rate
   *            - Number of channels
   *          You need to set.
   *
   *          If you would like to set codec binary path and bit length of audio data,
   *          You are able to set them by other type of this API.
   *          Default values of them are /mnt/sd0/BIN and AS_BITLENGTH_16(16bit).
   *
   */
  err_t initRecorder(
      uint8_t codec,  /**< Select compression code. AS_CODECTYPE_MP3 or AS_CODECTYPE_WAV */
      const char *codec_path, /**< Set DSP Binary path. Maximum length is 24 bytes.*/
      uint32_t fs,    /**<Set sampling rate. AS_SAMPLINGRATE_XXXXX. */
      uint8_t channel /**< Set channnel number. AS_CHANNEL_MONO, AS_CHANNEL_STEREO, AS_CHANNEL_4CH, or etc...  */
  );

  /**
   * @brief Initialize recorder
   *
   * @details This function initializes and sets Recorder action.
   *          When recorder do not start, you can call it as many times as you like.
   *
   *          By this function,
   *            - Compression codec
   *            - Sampling rate
   *            - Number of channels
   *          You need to set.
   *
   *          If you would like to set codec binary path and bit length of audio data,
   *          You are able to set them by other type of this API.
   *          Default values of them are /mnt/sd0/BIN and AS_BITLENGTH_16(16bit).
   *
   */
  err_t initRecorder(
      uint8_t codec,  /**< Select compression code. AS_CODECTYPE_MP3 or AS_CODECTYPE_WAV */
      const char *codec_path, /**< Set DSP Binary path. Maximum length is 24 bytes.*/
      uint32_t fs,    /**<Set sampling rate. AS_SAMPLINGRATE_XXXXX. */
      uint8_t bitlen, /**< Set bit length. AS_BITLENGTH_16 or AS_BITLENGTH_24 */
      uint8_t channel /**< Set channnel number. AS_CHANNEL_MONO, AS_CHANNEL_STEREO, AS_CHANNEL_4CH, or etc...  */
  );

  /**
   * @brief Start Player
   *
   * @details This function starts Player.
   *          Once you call this function, the Player will be in the active state,
   *          so you can not call it until you call StopPlayer.
   *
   *          When Player is started, it starts reading the data for the Access Unit[
   *          from the stream data buffer.
   *          Therefore, in order to start Player, it is necessary to supply
   *          Stream Data to the stream buffer beforehand.
   *          Be sure to call writeFrames before startPlay.
   *          If you execute without calling * writeFrames, ES_UNDER_FLOW_ERR will be occurs.
   *
   */
  err_t startPlayer(
      PlayerId id    /**< Select Player ID. */
  );

  /**
   * @brief Start Recorder
   *
   * @details This function starts Recorder.
   *          Once you call this function, the Recorder will be in the active state,
   *          so you can not call it until you call StopRecorder.
   *          And, in the case of WAV data, it is necessary to create a Wav Header
   *          at the beginning of the file, you need to call writeWavHeader function at first.
   *
   */
  err_t startRecorder(void);

  /**
   * @brief Stop Player
   *
   * @details This function stops Player.
   *          The function can be called only when called startPlayer and changed to the Playing state.
   *
   *          When stop player, it read Stream Data until the last frame, and stops.
   *          The next API will not be accepted until the audio output
   *          stops completely. (it takes about 100 ms).
   *
   */
  err_t stopPlayer(
      PlayerId id /**< Select Player ID. */
  );

  /**
   * @brief Stop Player (mode specified)
   *
   * @details This function stops Player with specified mode.
   *
   *          You can set stop mode. If you would like to stop player immediately,
   *          set mode to AS_STOPPLAYER_NORMAL. If you don't set mode, player stops
   *          after all of audio data in buffer is going out.
   *
   */
  err_t stopPlayer(
      PlayerId id, /**< Select Player ID. */
      uint8_t mode /**< Stop mode. AS_STOPPLAYER_NORMAL, AS_STOPPLAYER_ESEND */
  );

  /**
   * @brief Stop Recorder
   *
   * @details This function stops Recorder.
   *          The function can be called only when called startRecorder and changed to the Recording state.
   *
   *          When stop the Recorder, stop the audio capture and write until the last captured audio data.
   *          If this function return, the recording process had ended.
   *
   */
  err_t stopRecorder(void);

  /**
   * @brief Set Beep Sound
   *
   * @details This function sets beep sound.
   *          Beep sound On / Off, volume, pitch (frequency) can be configured.
   *          It can call on PlayerMode or ReadyMode status.
   *
   */
  err_t setBeep(
      char  enable, /**< Set beep sound On/Off. enable(On) = 1, disable(Off) = 0. */
      short volume, /**< Set beep sound volume. -90(db) - 0(db) can be set. 0db is Maximum amplitude. */
      short frequency /**< Set beep sound. frequency (pitch). Set the frequency value as it is. (example, 1000 is 1kHz sound) */
  );

  /**
   * @brief Set Player Volume
   *
   * @details This function can set the volume when playing the player.
   *         It can be called on PlayerMode.
   *
   */
  err_t setVolume(
      int volume /**< Set the master volume. Range of volume is -1020(-102db) - 120(12db). A value larger than 0 may distort the sound. */
  );

  /**
   * @brief Set Player Volume
   *
   * @details This function can set the volume when playing the player.
   *         It can be called on PlayerMode.
   *
   */
  err_t setVolume(
      int master,  /**< Set the master volume. Range of volume is -1020(-102db) - 120(12db). A value larger than 0 may distort the sound. */
      int player0, /**< Set the player0 volume. Range of volume is -1020(-102db) - 120(12db). This value is before Mixing. */
      int player1  /**< Set the player1 volume. Range of volume is -1020(-102db) - 120(12db). This value is before Mixing.*/
  );

  /**
   * @brief Set Player L/R Gain
   *
   * @details This function can set the Left and Right channel gain for Player playback.
   *          It can be executed with PlayerMode.
   *          If you do not call this, the sound is original.
   *
   */
  err_t setLRgain(
      PlayerId id,          /**< Select Player ID. */
      unsigned char l_gain, /**< Set left gain. 0(%) - 200(%) can be set. 100% is orignal and a value larger than 100 may distort the sound. */
      unsigned char r_gain  /**< Set right gain. 0(%) - 200(%) can be set. 100% is orignal and a value larger than 100 may distort the sound. */
  );

  /** APIs for Player Mode */

  /**
   * @brief Write Stream Data from a file to FIFO by some frames.(now 5 frames)
   *
   * @details This function writes from the audio file specified by the File class
   *          to the Stream  data FIFO in the Audio library.
   *          It writes for several frames data (now five frames).
   *          It can be called on PlayerMode.
   *
   *          This FIFO is cleared when calling StopPlayer or setReadyMode.
   *
   *          During music playback, please call this function periodically.
   *          When an error occurs, you should error handling as properly
   *
   */
  err_t writeFrames(
      PlayerId id, /**< Select Player ID. */
      File& myfile /**< Specify an instance of the File class of the audio file. */
  );

  /**
   * @brief Write Stream Data from a file to FIFO by some frames.(now 5 frames)
   *
   * @details This function writes from the audio file specified by the File class
   *          to the Stream  data FIFO in the Audio library.
   *          It writes for several frames data (now five frames).
   *          It can be called on PlayerMode.
   *
   *          This FIFO is cleared when calling StopPlayer or setReadyMode.
   *
   *          During music playback, please call this function periodically.
   *          When an error occurs, you should error handling as properly
   *
   */
  err_t writeFrames(
      PlayerId id, /**< Select Player ID. */
      int fd       /**< file pointer of the audio file. */
  );

  /**
   * @brief Write Stream Data from buffer
   *
   * @details This function writes from the buffer
   *          to the Stream  data FIFO in the Audio library.
   *          It writes for several frames data (now five frames).
   *          It can be called on PlayerMode.
   *
   *          This FIFO is cleared when calling StopPlayer or setReadyMode.
   *
   *          During music playback, please call this function periodically.
   *          When an error occurs, you should error handling as properly
   *
   */

  err_t writeFrames(
      PlayerId id,        /** Select Player ID. */
      uint8_t *data,      /** Buffer address of the audio data. */
      uint32_t write_size /** Size of the audio data. */
  );

  /** APIs for Recorder Mode */

  /**
   * @brief Write WAV Header.
   *
   * @details This function should call when file format is WAV file recording.
   *          When codec of InitRecoder is "wav", be sure to call it before StartRecoder.
   *          Do not call it if other codecs are selected.
   *
   */
  err_t writeWavHeader(
      File& myFile/**< Specify an instance of the File class of the audio file. */
  );

  /**
   * @brief Read Stream Data from FIFO to a file by some frames.(now 5 frames)
   *
   * @details This function reads the generated Stream data from the Stream FIFO
   *          into the file specified by the File class.
   *          It reads for several frames data (now five frames).
   *          It can be called on RecorderMode.
   *
   *          During sound recording, please call this function periodically.
   *
   */
  err_t readFrames(
      File& myFile/**< Specify an instance of the File class of the audio file. */
  );

  /**
   * @brief Close Outputfile
   *
   * @details This function do closing processing on the file in which stream is written.
   *          Be sure to call it after calling StopRecorder.
   *          It can be called on RecorderMode.
   *
   */
  err_t closeOutputFile(
      File& myFile/**< Specify an instance of the File class of the audio file. */
  );


  /**
   * @brief Read Stream Data from FIFO to a file by some frames.(now 5 frames)
   *
   * @details This function reads the generated Stream data from the Stream FIFO
   *          into the specified buffer area.
   *
   *          This function is for you want to process sounds in applications.
   *
   *          It reads for several frames data (now five frames).
   *          It can be called on RecorderMode.
   *
   *          During sound recording, please call this function periodically.
   *
   */
  err_t readFrames(
      char*     p_buffer,    /**< Address of buffer area. */
      uint32_t  buffer_size, /**< Buffer size.(byte) */
      uint32_t* read_size    /**< Read size.(byte) */
  );

  /**
   * @brief Set Rendering clock mode.
   *
   * @details This function sets the internal data rate mode of rendering to
   *          Normal or High Resolution.
   *
   *          The internal data rate Normal indicates "fs = 48 kHz" and
   *          High Resolution indicates "fs = 192 kHz".
   *
   *          Please call the function in Ready Mode.
   *          Ready Mode is either after calling bigin () or
   *          after calling setReadyMode ().
   *
   *          The default when not calling is Normal.
   *
   */
  err_t setRenderingClockMode(
      AsClkMode mode /**< Mode of rendering clock. */
  );

 /**
   * @brief Get recording ES size.
   *
   * @details This function gets recording elementary stream size.
   *
   */
   int getRecordingSize() {
     return m_es_size;
   }

private:

  /**
   * To avoid create multiple instance
   */

  AudioClass()
    : m_player0_simple_fifo_buf(NULL)
    , m_player1_simple_fifo_buf(NULL)
    , m_attention_callback(NULL)
  {}
  AudioClass(const AudioClass&);
  AudioClass& operator=(const AudioClass&);
  ~AudioClass() {}

  char m_es_player0_buf[FIFO_FRAME_SIZE];
  char m_es_player1_buf[WRITE_FIFO_FRAME_SIZE];

  CMN_SimpleFifoHandle m_player0_simple_fifo_handle;
  CMN_SimpleFifoHandle m_player1_simple_fifo_handle;
  uint32_t *m_player0_simple_fifo_buf;
  uint32_t *m_player1_simple_fifo_buf;

  AsPlayerInputDeviceHdlrForRAM m_player0_input_device_handler;
  AsPlayerInputDeviceHdlrForRAM m_player1_input_device_handler;

  AsRecorderOutputDeviceHdlr    m_output_device_handler;
  int                           m_es_size;
  WAVHEADER                     m_wav_format;
  int                           m_codec_type;

  AudioAttentionCb m_attention_callback;

  /* Private Functions */

  /* Functions for initialization on begin/end */
  err_t begin_manager(void);
  err_t begin_player(void);
  err_t begin_recorder(void);

  err_t end_manager(void);
  err_t end_player(void);
  err_t end_recorder(void);

  err_t activateAudio(void);

  err_t powerOn(void);
  err_t powerOff(void);

  /* Functions for initialization Encoder */
	
  err_t initMicFrontend(uint8_t ch_num, uint8_t bit_length, uint16_t sample);
  err_t init_recorder_wav(AudioCommand* command, uint32_t sampling_rate, uint8_t bit_length, uint8_t channel_number);
  err_t init_recorder_mp3(AudioCommand* command, uint32_t sampling_rate, uint8_t bit_length, uint8_t channel_number);
  err_t init_recorder_opus(AudioCommand* command, uint32_t sampling_rate, uint8_t bit_length, uint8_t channel_number);
  err_t init_recorder_pcm(AudioCommand* command, uint32_t sampling_rate, uint8_t bit_length, uint8_t channel_number);

  /* Functions for initialization on player mode. */
  err_t set_output(uint8_t device, uint8_t sp_drv);

  err_t write_fifo(int, char*, uint32_t, CMN_SimpleFifoHandle*);
  err_t write_fifo(File&, char*, uint32_t, CMN_SimpleFifoHandle*);

  /* Functions for initialization on recorder mode. */
  err_t set_mic_map(uint8_t map[AS_MIC_CHANNEL_MAX]);
  err_t init_mic_gain(int, int);

  /* Functions for initialization on through mode. */
  err_t send_set_through(void);

  bool check_decode_dsp(uint8_t codec_type, const char *path);
  bool check_encode_dsp(uint8_t codec_type, const char *path, uint32_t fs);
};

extern AudioClass Audio;

/** @} audio */

// #endif //__cplusplus
#endif //Audio_h

