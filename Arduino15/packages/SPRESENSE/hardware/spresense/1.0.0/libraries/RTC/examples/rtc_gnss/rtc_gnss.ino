/*
 *  rtc_gnss.ino - Example for setting RTC via GNSS
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
 *  This is a example for setting RTC via GNSS.
 */

#include <RTC.h>
#include <GNSS.h>

SpGnss Gnss;

#define MY_TIMEZONE_IN_SECONDS (9 * 60 * 60) // JST

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

  Serial.println("Example for GPS clock");

  // Initialize RTC at first
  RTC.begin();

  // Initialize and start GNSS library
  int ret;
  ret = Gnss.begin();
  assert(ret == 0);

  ret = Gnss.start();
  assert(ret == 0);
}

void loop()
{
  // Wait for GNSS data
  if (Gnss.waitUpdate()) {
    SpNavData  NavData;

    // Get the UTC time
    Gnss.getNavData(&NavData);
    SpGnssTime *time = &NavData.time;

    // Check if the acquired UTC time is accurate
    if (time->year >= 2000) {
      RtcTime now = RTC.getTime();
      // Convert SpGnssTime to RtcTime
      RtcTime gps(time->year, time->month, time->day,
                  time->hour, time->minute, time->sec, time->usec * 1000);
#ifdef MY_TIMEZONE_IN_SECONDS
      // Set the time difference
      gps += MY_TIMEZONE_IN_SECONDS;
#endif
      int diff = now - gps;
      if (abs(diff) >= 1) {
        RTC.setTime(gps);
      }
    }
  }

  // Display the current time every a second
  updateClock();
}

