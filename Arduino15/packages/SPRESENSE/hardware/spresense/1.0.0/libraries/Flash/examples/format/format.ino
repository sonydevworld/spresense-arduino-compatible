/*
 *  format.ino - Flash format sample application
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

/**
 * @file format.ino
 * @author Sony Semiconductor Solutions Corporation
 * @brief Flash format sample application.
 */

#include <Arduino.h>
#include <Flash.h>

/**
 * @brief Write to the file and read from the file.
 *
 * @details The file is located on the Flash.
 */
void setup()
{
  /* Open serial communications and wait for port to open */
  Serial.begin(115200);
  while (!Serial) {
    ; /* wait for serial port to connect. Needed for native USB port only */
  }

  /* Notice to format SPI-Flash */
  Serial.println("WARNING: Formatting will erase ALL data on SPI-Flash mounted at /mnt/spif.");
  Serial.println("WARNING: EEPROM data emulated using SPI-Flash will be also clear.");
  Serial.println();
  Serial.println("It takes about 40 seconds to format.");
  Serial.println("Please do not turn off the power supply until formatting is complete.");
  Serial.println("During formatting, it turns 4 LEDs on. If it's complete, LEDs are blinking.");
  Serial.println();
  Serial.println("Do you want to format? (y/n)");
  Serial.println("To format SPI-Flash, input 'y'.");
  Serial.println();

  while (!Serial.available());
  int c = Serial.read();
  while (Serial.available()) {
    Serial.read(); // Discard inputs except for 1st character
  }

  if ((c != 'y') && (c != 'Y')) {
    Serial.println("Canceled.");
    return;
  }

  /* Start to format */
  ledOn(LED0); ledOn(LED1); ledOn(LED2); ledOn(LED3);
  Serial.println("Format start...");
  int ret = Flash.format();

  if (ret) {
    Serial.println("Failure!!");
  } else {
    Serial.println("Success!!");
  }

  /* Finish to format */
  while (1) {
    ledOff(LED0); ledOff(LED1); ledOff(LED2); ledOff(LED3);
    delay(500);
    ledOn(LED0); ledOn(LED1); ledOn(LED2); ledOn(LED3);
    delay(500);
  }
}

/**
 * @brief Run repeatedly.
 *
 * @details Does not do anything.
 */
void loop() {

}
