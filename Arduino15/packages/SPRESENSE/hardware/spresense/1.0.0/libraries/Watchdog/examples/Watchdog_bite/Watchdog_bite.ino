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
 *  This is a test app for the watchdog library.
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
  Watchdog.begin();
}

void loop() {
  static int delay_ms = 1000;

  /**
   * Start a watchdog
   */
  Watchdog.start(2000);

  /**
   * Wait for next loop
   * (If delay_ms exceeds delay_ms, watchdog will bite)
   */
  Serial.print("Sleep ");
  Serial.print(delay_ms);
  Serial.println("ms");
  usleep(1000 * delay_ms);

  /**
   * Check remain time for watchdog bite
   */
  Serial.print(Watchdog.timeleft());
  Serial.println("ms left for watchdog bite");

  /**
   * Kick a watchdog
   */
  Serial.println("Kick!");
  Watchdog.kick();

  /**
   * Increase wati time
   */
  delay_ms += 100;

  /**
   * Stop a watchdog
   */
  Watchdog.stop();
}
