/*
 *  LteNtpClient.ino - Example for NTP client using LTE
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
 *  This sketch connects to a NTP server via LTE.
 */

// libraries
#include <NTPClient.h>
#include <LTE.h>

// APN data
#define LTE_APN       "apn"      // replace your APN
#define LTE_USER_NAME "user"     // replace with your username
#define LTE_PASSWORD  "password" // replace with your password

#define NTP_SERVER_NAME "ntp.nict.jp"
#define TIME_OFFSET     (9 * 60 * 60) // time offset in seconds(This example is JST)
#define UPDATE_INTERVAL 60000         // update interval in milliseconds

// initialize the library instance
LTE lteAccess;
LTEUDP ntpUDP;

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, NTP_SERVER_NAME, TIME_OFFSET, UPDATE_INTERVAL);

void setup()
{
  // initialize serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Starting NTP client.");

  // If your SIM has PIN, pass it as a parameter of begin() in quotes
  while (true) {
    if (lteAccess.begin() == LTE_SEARCHING) {
      if (lteAccess.attach(LTE_APN, LTE_USER_NAME, LTE_PASSWORD) == LTE_READY) {
        Serial.println("attach succeeded.");
        break;
      }
      Serial.println("An error occurred, shutdown and try again.");
      lteAccess.shutdown();
      sleep(1);
    }
  }

  timeClient.begin();
}

void loop() {
  timeClient.update();

  Serial.println(timeClient.getFormattedTime());

  sleep(1);
}
