/*
 *  number_recognition.ino - hand written number recognition sample application
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
 * @file number_recognition.ino
 * @author Sony Semiconductor Solutions Corporation
 * @brief DNNRT sample application.
 *
 * This sample uses the network model file (.nnb) and recognize image
 * in pgm (portable greyscale map) file. Both of requred files should be
 * placed at the SD card. And adjust file path (nnbfile and pgmfile) if
 * needed.
 *
 * A sample NNB file `network.nnb` is provided in the same directory as this
 * sketch. See below for instructions on how to create this NNB file.
 * https://developer.sony.com/develop/spresense/docs/arduino_developer_guide_en.html#_dnnrt_library
 */

#include <SDHCI.h>
#include <NetPBM.h>
#include <DNNRT.h>

DNNRT dnnrt;
SDClass SD;

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  File nnbfile = SD.open("network.nnb");
  if (!nnbfile) {
    Serial.print("nnb not found");
    return;
  }
  int ret = dnnrt.begin(nnbfile);
  if (ret < 0) {
    Serial.println("Runtime initialization failure.");
    if (ret == -16) {
      Serial.print("Please install bootloader!");
      Serial.println(" or consider memory configuration!");
    } else {
      Serial.println(ret);
    }
    return;
  }

  // Image size for this network model is 28 x 28.

  File pgmfile("number4.pgm");
  NetPBM pgm(pgmfile);

  unsigned short width, height;
  pgm.size(&width, &height);

  DNNVariable input(width * height);
  float *buf = input.data();
  int i = 0;

  /*
   * Normalize pixel data into between 0.0 and 1.0.
   * PGM file is gray scale pixel map, so divide by 255.
   * This normalization depends on the network model.
   */

  for (int x = 0; x < height; x++) {
    for (int y = 0; y < width; y++) {
      buf[i] = float(pgm.getpixel(x, y)) / 255.0;
      i++;
    }
  }

  dnnrt.inputVariable(input, 0);
  dnnrt.forward();
  DNNVariable output = dnnrt.outputVariable(0);

  /*
   * Get index for maximum value.
   * In this example network model, this index represents a number,
   * so you can determine recognized number from this index.
   */

  int index = output.maxIndex();
  Serial.print("Image is ");
  Serial.print(index);
  Serial.println();
  Serial.print("value ");
  Serial.print(output[index]);
  Serial.println();

  dnnrt.end();
}

void loop() {
  // put your main code here, to run repeatedly:

}
