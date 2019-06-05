/*
 *  UsbMsc.ino - Example to Open SD Card on the PC as USB Mass Storage
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

#include <SDHCI.h>
SDClass SD;

void setup() {
  Serial.begin(115200);

  /* Initialize SD */
  while (!SD.begin()) {
    ; /* wait until SD card is mounted. */
  }

  /* Start USB MSC */
  if (SD.beginUsbMsc()) {
    Serial.println("USB MSC Failure!");
  } else {
    Serial.println("*** USB MSC Prepared! ***");
    Serial.println("Insert SD and Connect Extension Board USB to PC.");
  }
}

void loop() {
}
