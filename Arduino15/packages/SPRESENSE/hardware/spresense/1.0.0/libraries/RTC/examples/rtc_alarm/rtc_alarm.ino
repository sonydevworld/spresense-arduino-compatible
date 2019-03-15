/*
 *  rtc_alarm.ino - Example for RTC alarm
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
 *  This is a example for RTC alarm.
 *  This example set the RTC alarm and blink a LED every 5 seconds.
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

void alarmExpired(void)
{
  static int cnt = 0;

  RtcTime now = RTC.getTime();

  // Set the RTC alarm every 5 seconds
  RtcTime alm = now + 5;
  RTC.setAlarm(alm);

  printClock(now);

  // Toggle LED0
  if (cnt++ % 2) {
    ledOff(LED0);
  } else {
    ledOn(LED0);
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Example for alarm every 5 seconds");

  // Initialize RTC at first
  RTC.begin();

  // Set the RTC alarm handler
  RTC.attachAlarm(alarmExpired);

  // Set the temporary RTC time
  RtcTime compiledDateTime(__DATE__, __TIME__);
  RTC.setTime(compiledDateTime);

  RtcTime now = RTC.getTime();
  printClock(now);

  // Set the RTC alarm after 5 seconds
  now += 5;
  RTC.setAlarm(now);
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
}

