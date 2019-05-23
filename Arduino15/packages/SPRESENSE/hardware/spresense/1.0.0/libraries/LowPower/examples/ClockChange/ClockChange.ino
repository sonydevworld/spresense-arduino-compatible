/*
 *  ClockChange.ino - Example for Clock Change dynamically
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

#include <LowPower.h>
#include <RTC.h>

volatile int alarmInt = 0;

void alarmExpired(void)
{
  alarmInt = 1;
}

void printCounter()
{
  int counter = 0;

  /* Print counter between 5 seconds */
  RTC.setAlarmSeconds(5);

  while (1) {
    counter++;
    if (alarmInt) {
      alarmInt = 0;
      break;
    }
  }
  Serial.print("counter= ");
  Serial.println(counter);
}

void printClockMode()
{
  clockmode_e mode = LowPower.getClockMode();

  Serial.println("--------------------------------------------------");
  Serial.print("clock mode: ");
  switch (mode) {
  case CLOCK_MODE_156MHz: Serial.println("156MHz"); break;
  case CLOCK_MODE_32MHz:  Serial.println("32MHz"); break;
  case CLOCK_MODE_8MHz:   Serial.println("8MHz"); break;
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  // Initialize RTC at first
  RTC.begin();

  // Set the RTC alarm handler
  RTC.attachAlarm(alarmExpired);

  // Initialize LowPower library
  LowPower.begin();
}

void loop()
{
  // Set the highest clock mode
  LowPower.clockMode(CLOCK_MODE_156MHz);
  printClockMode();
  printCounter();

  // Set the middle clock mode
  LowPower.clockMode(CLOCK_MODE_32MHz);
  printClockMode();
  printCounter();

  // Set the lowest clock mode
  LowPower.clockMode(CLOCK_MODE_8MHz);
  printClockMode();
  printCounter();
}

