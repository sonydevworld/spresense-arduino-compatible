/*
 *  recorder_wav.ino - Recorder example application for WAV(PCM)
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

#include <SDHCI.h>
#include <Audio.h>

#include <arch/board/board.h>

SDClass theSD;
AudioClass *theAudio;

File myFile;

bool ErrEnd = false;

/**
 * @brief Audio attention callback
 *
 * When audio internal error occurc, this function will be called back.
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
 * @brief Setup recording of mp3 stream to file
 *
 * Select input device as microphone <br>
 * Initialize filetype to stereo wav with 48 Kb/s sampling rate <br>
 * Open "Sound.wav" file in write mode
 */

static const int32_t recoding_frames = 400;
static const int32_t recoding_size = recoding_frames*3072; /* 2ch, 16bit, 768sample */

void setup()
{
  theAudio = AudioClass::getInstance();

  theAudio->begin(audio_attention_cb);

  puts("initialization Audio Library");

  /* Select input device as microphone */
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);

  /*
   * Initialize filetype to stereo wav with 48 Kb/s sampling rate
   * Search for WAVDEC codec in "/mnt/sd0/BIN" directory
   */
  theAudio->initRecorder(AS_CODECTYPE_WAV,"/mnt/sd0/BIN",AS_SAMPLINGRATE_48000,AS_CHANNEL_STEREO);
  puts("Init Recorder!");

  /* Open file for data write on SD card */
  myFile = theSD.open("Sound.wav", FILE_WRITE);
  /* Verify file open */
  if (!myFile)
    {
      printf("File open error\n");
      exit(1);
    }

  theAudio->writeWavHeader(myFile);
  puts("Write Header!");

  theAudio->startRecorder();
  puts("Recording Start!");

}

void loop() 
{
  err_t err;
  /* recording end condition */
  if (theAudio->getRecordingSize() > recoding_size)
    {
      theAudio->stopRecorder();
      sleep(1);
      err = theAudio->readFrames(myFile);

      goto exitRecording;
    }


  /* Read frames to record in file */
  err = theAudio->readFrames(myFile);

  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("File End! =%d\n",err);
      theAudio->stopRecorder();
      goto exitRecording;
    }

  if (ErrEnd)
    {
      printf("Error End\n");
      theAudio->stopRecorder();
      goto exitRecording;
    }

  /* This sleep is adjusted by the time to write the audio stream file.
     Please adjust in according with the processing contents
     being processed at the same time by Application.
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



