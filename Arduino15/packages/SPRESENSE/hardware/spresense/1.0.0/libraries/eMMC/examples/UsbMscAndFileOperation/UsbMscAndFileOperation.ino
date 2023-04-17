/*
 *  UsbMscAndFileOperation.ino - Example to eMMC File access and USB MSC
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
 */

#include <eMMC.h>

#define EMMC_POWER_PIN 26

void setup() {
  Serial.begin(115200);

  /* Initialize eMMC */
  eMMC.begin(EMMC_POWER_PIN);
}

void loop() {

  Serial.println(">>> Start File Operation");

  // Record counter into COUNTER.TXT on eMMC
  saveCounter();

  // Display eMMC File List
  listeMMCFile();

  Serial.println("<<< Finish File Operation");

  delay(1000);

  Serial.println(">>> Start USB Mass Storage Operation");

  // Start USB Mass Storage
  if (eMMC.beginUsbMsc()) {
    Serial.println("UsbMsc connect error");
  }

  Serial.println("Finish USB MSC? (y/n)");
  while (true) {
    while (!Serial.available());
    if(Serial.read() == 'y'){
      while (Serial.available()) {
        Serial.read(); // dummy read to empty input buffer
      }
      break;
    }
  }

  // Finish USB Mass Storage
  if (eMMC.endUsbMsc()) {
    Serial.println("UsbMsc disconnect error");
  }
  Serial.println("<<< Finish USB Mass Storage Operation");

  delay(1000);
}

void listeMMCFile() {
  Serial.println("Size\tFilename");
  Serial.println("----\t--------");
  File root = eMMC.open("");
  listDirectory(root);
}

void listDirectory(File dir) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }
    if (entry.isDirectory()) {
      listDirectory(entry); // recursive call
    } else {
      Serial.print(entry.size(), DEC);
      Serial.print("\t");
      Serial.println(entry.name());
    }
    entry.close();
  }
}

void saveCounter(void) {
  static int counter = 0;

  File counterFile = eMMC.open("COUNTER.TXT", FILE_WRITE);

  if (counterFile) {
    Serial.print("Counter=");
    Serial.println(counter);
    counterFile.println(counter);
    counterFile.close();
    counter++;
  } else {
    Serial.println("file open error");
  }
}

