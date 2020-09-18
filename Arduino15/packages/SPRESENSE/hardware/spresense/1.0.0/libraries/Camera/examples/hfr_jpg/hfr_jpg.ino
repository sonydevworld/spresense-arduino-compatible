/*
 *  hfr_jpg.ino - Shooting 5 seconds JPEG sequence with QVGA 120FPS
 *  Copyright 2019 Sony Semiconductor Solutions Corporation
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
 *
 *  This is a test app for the camera library.
 *  This library can only be used on the Spresense with the FCBGA chip package.
 */

#include <SDHCI.h>
#include <stdio.h>  /* for sprintf */

#include <Camera.h>
#include "led.h"

#define BAUDRATE                (115200)
#define FRAMES_NUM              (600) /* 120FPS * 5 seconds */
#define FILENAME_LEN            (16)

SDClass  theSD;
int frame_count = 0;

/**
 * Print error message
 */

void printError(enum CamErr err)
{
  Serial.print("Error: ");
  switch (err)
    {
      case CAM_ERR_NO_DEVICE:
        Serial.println("No Device");
        break;
      case CAM_ERR_ILLEGAL_DEVERR:
        Serial.println("Illegal device error");
        break;
      case CAM_ERR_ALREADY_INITIALIZED:
        Serial.println("Already initialized");
        break;
      case CAM_ERR_NOT_INITIALIZED:
        Serial.println("Not initialized");
        break;
      case CAM_ERR_NOT_STILL_INITIALIZED:
        Serial.println("Still picture not initialized");
        break;
      case CAM_ERR_CANT_CREATE_THREAD:
        Serial.println("Failed to create thread");
        break;
      case CAM_ERR_INVALID_PARAM:
        Serial.println("Invalid parameter");
        break;
      case CAM_ERR_NO_MEMORY:
        Serial.println("No memory");
        break;
      case CAM_ERR_USR_INUSED:
        Serial.println("Buffer already in use");
        break;
      case CAM_ERR_NOT_PERMITTED:
        Serial.println("Operation not permitted");
        break;
      default:
        break;
    }
}


/**
 * Callback from Camera library when video frame is captured.
 */

void CamCB(CamImage img)
{

  /* Check the img instance is available or not. */

  if (img.isAvailable())
    {
      led_update(frame_count++);

      if (frame_count > FRAMES_NUM)
        {
          return;
        }

      /* Create file name */
    
      char filename[FILENAME_LEN] = {0};
      snprintf(filename, FILENAME_LEN, "HFR%04d.JPG", frame_count);

      Serial.print("Save taken picture as ");
      Serial.print(filename);
      Serial.println("");

      /* Remove the old file with the same file name as new created file,
       * and create new file.
       */

      theSD.remove(filename);
      File myFile = theSD.open(filename, FILE_WRITE);
      myFile.write(img.getImgBuff(), img.getImgSize());
      myFile.close();
    }
  else
    {
      Serial.print("Failed to get video stream image\n");
    }
}

/**
 * @brief Initialize camera
 */
void setup()
{
  CamErr err;

  /* Open serial communications and wait for port to open */

  Serial.begin(BAUDRATE);
  while (!Serial)
    {
      ; /* wait for serial port to connect. Needed for native USB port only */
    }

  /* Initialize SD */
  while (!theSD.begin())
    {
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
    }

  /* Shooting JPEG sequence without frame's gap requires multiple buffers. */

  Serial.println("Prepare camera");
  err = theCamera.begin(2,
                        CAM_VIDEO_FPS_120,
                        CAM_IMGSIZE_QVGA_H,
                        CAM_IMGSIZE_QVGA_V,
                        CAM_IMAGE_PIX_FMT_JPG);
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

  /* Start video stream.
   * If received video stream data from camera device,
   *  camera library call CamCB.
   */

  err = theCamera.startStreaming(true, CamCB);
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

  led_init(FRAMES_NUM);
  Serial.println("Start streaming");
}

/**
 * @brief No procedure in shooting JPEG sequence. At last, finalize Camera library.
 */

void loop()
{
  if (frame_count > FRAMES_NUM)
    {
      Serial.println("Stop streaming");
      theCamera.startStreaming(false);
      theCamera.end();
      frame_count = 0; /*For prevention of repeating end process */
    }
}
