/*
 *  rtc_simple.ino - Example for RTC simple clock
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
 *  This is a example for RTC simple clock.
 *  This example outputs the current clock to the serial monitor every a second.
 *  If your board is connected to PC linux environment, you can set PC time to
 *  RTC on the board by below command.
 *   $ date +T%s > /dev/ttyUSB0  (ex. TZ=GMT/UTC)
 *   $ date --date='9 hours' +T%s > /dev/ttyUSB0  (ex. TZ=JST/UTC+0900)
 */

#include <RTC.h>

#define TIME_HEADER 'T' // Header tag for serial time sync message

void printClock(RtcTime &rtc)
{
  printf("%04d/%02d/%02d %02d:%02d:%02d\n",
         rtc.year(), rtc.month(), rtc.day(),
         rtc.hour(), rtc.minute(), rtc.second());
}

void updateClock()
{
  static RtcTime old;
  RtcTime now = RTC.getTime();

  // Display only when the second is updated
  if (now != old) {
    printClock(now);
    old = now;
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Example for RTC simple clock");

  // Initialize RTC at first
  RTC.begin();

  // Set the temporary RTC time
  RtcTime compiledDateTime(__DATE__, __TIME__);
  RTC.setTime(compiledDateTime);
}

void loop()
{
  // Synchronize with the PC time
  if (Serial.available()) {
    if(Serial.find(TIME_HEADER)) {
      uint32_t pctime = Serial.parseInt();
      RtcTime rtc(pctime);
      RTC.setTime(rtc);
    }
  }

  // Display the current time every a second
  updateClock();
}

