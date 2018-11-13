/*
 *  Watchdog_bite.ino - Watchdog sample application
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

#include <Watchdog.h>

#define BAUDRATE                (115200)

/**
 * @brief Initialize Watchdog
 */
void setup() {
  
  /* Open serial communications and wait for port to open */

  Serial.begin(BAUDRATE);
  while (!Serial)
    {
      ; /* wait for serial port to connect. Needed for native USB port only */
    }

  /**
   * Initialize a watchdog
   */
  Watchdog.begin(2000);

  /**
   * Start a watchdog
   */
  Watchdog.start();
}

void loop() {
  static int delay_ms = 1000;
  
  Serial.println("Kick!");
  Watchdog.kick();

  Serial.print("Sleep ");
  Serial.print(delay_ms);
  Serial.println("ms");
  usleep(1000 * delay_ms);

  delay_ms += 100;
}
