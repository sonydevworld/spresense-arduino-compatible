/*
 *  interval_write.ino - eMMC read/write on interval sample application
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

/**
 * @file interval_write.ino
 * @author Sony Semiconductor Solutions Corporation
 * @brief eMMC read/write on interval sample application.
 */

#include <Arduino.h>
#include <File.h>
#include <eMMC.h>

#define EMMC_POWER_PIN 26

static const int loop_count = 3;

File myFile; /**< File object */ 

/**
 * @brief Write to the file and read from the file on interval.
 * 
 * @details The file is located on the eMMC.
 */
void setup() {

  /* Open serial communications and wait for port to open */
  Serial.begin(115200);
  while (!Serial) {
    ; /* wait for serial port to connect. Needed for native USB port only */
  }

}

/**
 * @brief Run repeatedly.
 * 
 * @details Reads and writes a certain number of times on interval.
 */
void loop() {

  for(int i = 0; i<loop_count; i++)
    {
      /* Initialize eMMC */
      eMMC.begin(EMMC_POWER_PIN);

      /* Create a new directory */
      eMMC.mkdir("dir/");

      /* Open the file. */
      myFile = eMMC.open("dir/test.txt", FILE_WRITE);

      /* If the file opened okay, write to it */
      if (myFile)
        {
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
      myFile = eMMC.open("dir/test.txt");

      if (myFile)
        {
          Serial.println("test.txt:");

          /* Read from the file until there's nothing else in it */
          while (myFile.available())
            {
              Serial.write(myFile.read());
            }
          /* Close the file */
          myFile.close();
        } else {
          /* If the file didn't open, print an error */
          Serial.println("error opening test.txt");
        }

      /* Finalize eMMC */
      eMMC.end();

      sleep(10);

    }

  Serial.println("Sample end.");
  while(1);

}
