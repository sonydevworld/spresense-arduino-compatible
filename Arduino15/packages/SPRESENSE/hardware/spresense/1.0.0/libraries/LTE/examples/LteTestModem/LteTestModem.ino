/*
 *  LteTestModem.ino - Example for obtaining modem information
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
 *  This sketch turns on the modem and gets IMEI and FW version.
 *
 *  Do not need a SIM card for this example.
 */

// libraries
#include <LTE.h>

// initialize the library instance
LTEModem modem;

// IMEI variable
String IMEI = "";
String VERSION = "";

void setup() {
  // initialize serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
      ; /* wait for serial port to connect. Needed for native USB port only */
  }

  // start modem test (reset and check response)
  Serial.println("Starting modem test...");
  if (LTE_IDLE == modem.begin()) {
    Serial.println("modem.begin() succeeded.");
  } else {
    Serial.println("ERROR, no modem answer.");
  }
}

void loop() {

  // IMEI of the modem
  IMEI = modem.getIMEI();
  Serial.println("IMEI: " + IMEI);

  //Firmware version of the modem.
  VERSION = modem.getFirmwareVersion();
  Serial.println("VERSION: " + VERSION);

  sleep(1);
}
