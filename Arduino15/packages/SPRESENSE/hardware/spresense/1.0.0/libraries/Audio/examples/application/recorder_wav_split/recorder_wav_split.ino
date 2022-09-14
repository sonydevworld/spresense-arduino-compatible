/*
 *  recorder_wav_split.ino - Recorder(recording to split files) example application for WAV(PCM)
 *  Copyright 2022 Sony Semiconductor Solutions Corporation
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
#include <Audio.h>

#define RECORD_FILE_NAME "Sound"
#define RECORD_DIR_NAME    "Rec"
#define DIR_PATH_LEN         32
#define FILE_PATH_LEN       128
static char dir_name[FILE_PATH_LEN];

SDClass theSD;
AudioClass *theAudio;

File myFile;

bool ErrEnd = false;

/**
 * @brief Audio attention callback
 *
 * When audio internal error occurs, this function will be called back.
 */

static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");
  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      ErrEnd = true;
   }
}

/**
 * @brief Setup recording of wav stream to file
 *
 * Select input device as microphone <br>
 * Initialize filetype to stereo wav with 48 Kb/s sampling rate <br>
 * Open RECORD_FILE_NAME file in write mode
 */

/* Sampling rate
 * Set 16000 or 48000
 */

static const uint32_t recoding_sampling_rate = 48000;

/* Number of input channels
 * Set either 1, 2, or 4.
 */

static const uint8_t  recoding_cannel_number = 2;

/* Audio bit depth
 * Set 16 or 24
 */

static const uint8_t  recoding_bit_length = 16;

/* Mic Gain */

static const uint8_t  mic_gain = 16;

/* Encoded date buffer size */

static const uint32_t enc_buf_size = 160*1024;

/* Recording time[second] */

static const uint32_t recoding_time = 60*60;

/* Recording split time[second] */

static const int32_t split_time = 10*60;

/* Bytes per second */

static const int32_t recoding_byte_per_second = recoding_sampling_rate *
                                                recoding_cannel_number *
                                                recoding_bit_length / 8;

/* Total recording size */

static const int32_t recoding_size = recoding_byte_per_second * recoding_time;

/* split recording size */

static const uint32_t split_size = recoding_byte_per_second * split_time;


/* Number of files in one directory */

static const uint32_t dir_file_num = 100;


/* Number of directories */

static uint16_t dir_count  = 0;

/* Number of files */

static uint16_t file_count = 0;

/**
 * @brief create recording directories
 */
 
static void create_dir()
{
  snprintf(dir_name, sizeof(dir_name), "%s%02d", RECORD_DIR_NAME, dir_count);

  if (!theSD.exists(dir_name))
    {
      theSD.mkdir(dir_name);
    }

  dir_count++;
}

/**
 * @brief create record file
 *
 * create the record file to SD card.
 */

static void create_file(File *record_file, uint16_t count)
{
  char file_path[FILE_PATH_LEN];

  snprintf(file_path, sizeof(file_path), "%s/%s%03d.wav", dir_name, RECORD_FILE_NAME, count);
  if (theSD.exists(file_path))
    {
      printf("Remove existing file [%s].\n", file_path);
      theSD.remove(file_path);
    }

  *record_file = theSD.open(file_path, FILE_WRITE);
  /* Verify file open */
  if (!myFile)
    {
      printf("File open error\n");
      exit(1);
    }

  printf("Open! [%s]\n", file_path);
}

void setup()
{
  Serial.begin(115200);
  
  /* Initialize SD */
  while (!theSD.begin())
    {
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
    }

  theAudio = AudioClass::getInstance();

  theAudio->begin(audio_attention_cb);

  puts("initialization Audio Library");

  /* Select input device as microphone */
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);

  /* If file writing fails, change the buffer size to a larger one.
     And if recording volume is small, change microphone gain. */
//  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC, mic_gain, enc_buf_size);

  /* Search for WAVDEC codec in "/mnt/sd0/BIN" directory */
  theAudio->initRecorder(AS_CODECTYPE_WAV,
                         "/mnt/sd0/BIN",
                         recoding_sampling_rate,
                         recoding_bit_length,
                         recoding_cannel_number);
  puts("Init Recorder!");

  create_dir();

  create_file(&myFile, file_count++);

  theAudio->writeWavHeader(myFile);
  puts("Write Header!");

  theAudio->startRecorder();
  puts("Recording Start!");

}

void loop() 
{
  err_t err;

  if (file_count >= (dir_file_num * dir_count))
    {
      create_dir();
    }

  /* recording end condition */
  if (theAudio->getRecordingSize() + split_size * (file_count - 1) > recoding_size)
    {
      theAudio->stopRecorder();
      sleep(1);
      err = theAudio->readFrames(myFile);

      goto exitRecording;      
    }
  else if ((uint32_t)theAudio->getRecordingSize() > split_size)
    {
      theAudio->closeOutputFile(myFile);
      myFile.close();
      create_file(&myFile, file_count++);

      theAudio->writeWavHeader(myFile);
      
      err = theAudio->readFrames(myFile);
    }
  else
    {
      err = theAudio->readFrames(myFile);
    }

  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("Error End! =%d\n",err);
      ErrEnd = true;
    }

  if (ErrEnd)
    {
      printf("Error End\n");
      theAudio->stopRecorder();
      goto exitRecording;
    }

  /* This sleep is adjusted by the time to write the audio stream file.
   * Please adjust in according with the processing contents
   * being processed at the same time by Application.
   *
   * The usleep() function suspends execution of the calling thread for usec
   * microseconds. But the timer resolution depends on the OS system tick time
   * which is 10 milliseconds (10,000 microseconds) by default. Therefore,
   * it will sleep for a longer time than the time requested here.
   */

//  usleep(10000);

  return;

exitRecording:

  theAudio->closeOutputFile(myFile);
  myFile.close();
  
  theAudio->setReadyMode();
  theAudio->end();
  
  puts("End Recording");
  exit(1);

}
