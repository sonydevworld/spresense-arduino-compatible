/*
 *  camera.ino - One minute interval time-lapse Camera
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
 *
 *  This is a test app for the camera library.
 *  This library can only be used on the Spresense with the FCBGA chip package.
 */

#include <SDHCI.h>

#include <Camera.h>
#include <arch/board/board.h>

SDClass theSD;

CameraClass theCamera;

/**
 * @brief Initialize camera
 */
void setup() {
  puts("Prepare camera");
  theCamera.begin();
  theCamera.initialize();
}

/**
 * @brief Save picture to file in one minute interval
 */
void loop() {
  static int  cnt = 0;
  static char filename[14] = {0};

  if (cnt > 999)
    {
      printf("Saved %d JPEG pictures\n", cnt);
      printf("Camera exit\n");
      exit(1);
    }

  sprintf(filename, "camera%03d.jpg", cnt);
  File myFile = theSD.open(filename, FILE_WRITE);

  printf("Requesting JPEG capture and save picture to SD card. Filename: %s\n", filename);
  theCamera.read(myFile);

  cnt++;

  /* One minute interval. */
  sleep(60);
}
