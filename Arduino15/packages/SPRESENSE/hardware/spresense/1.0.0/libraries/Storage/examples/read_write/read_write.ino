/*
 *  read_write.ino - Flash read/write sample application
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
 * @file read_write.ino
 * @author Sony Semiconductor Solutions Corporation
 * @brief Flash read/write sample application.
 */

#include <Arduino.h>
#include <File.h>
#include <Storage.h>

File myFile; /**< File object */ 

/**
 * @brief Write to the file and read from the file.
 * 
 * @details The file is located on the SD card.
 */
void setup() {

  /* Open serial communications and wait for port to open */
  Serial.begin(115200);
  while (!Serial) {
    ; /* wait for serial port to connect. Needed for native USB port only */
  }

  /* Create a new directory */
  Storage.mkdir("/mnt/spif/dir/");

  /* Open the file. Note that only one file can be open at a time,
     so you have to close this one before opening another. */
  myFile = Storage.open("/mnt/spif/dir/test.txt", FILE_WRITE);

  /* If the file opened okay, write to it */
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
    /* Close the file */
    myFile.close();
    Serial.println("done.");
  } else {
    /* If the file didn't open, print an error */
    Serial.println("error opening test.txt");
  }

  /* Re-open the file for reading */
  myFile = Storage.open("/mnt/spif/dir/test.txt");

  if (myFile) {
    Serial.println("test.txt:");

    /* Read from the file until there's nothing else in it */
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    /* Close the file */
    myFile.close();
  } else {
    /* If the file didn't open, print an error */
    Serial.println("error opening test.txt");
  }
}

/**
 * @brief Run repeatedly.
 * 
 * @details Does not do anything.
 */
void loop() {

}
