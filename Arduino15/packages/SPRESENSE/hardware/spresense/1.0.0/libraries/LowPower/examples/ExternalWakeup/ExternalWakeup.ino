/*
 *  ExternalWakeup.ino - Example for GPIO wakeup from cold sleep
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
 */

#include <LowPower.h>
#include <RTC.h>

const char* boot_cause_strings[] = {
  "Power On Reset with Power Supplied",
  "System WDT expired or Self Reboot",
  "Chip WDT expired",
  "WKUPL signal detected in deep sleep",
  "WKUPS signal detected in deep sleep",
  "RTC Alarm expired in deep sleep",
  "USB Connected in deep sleep",
  "Others in deep sleep",
  "SCU Interrupt detected in cold sleep",
  "RTC Alarm0 expired in cold sleep",
  "RTC Alarm1 expired in cold sleep",
  "RTC Alarm2 expired in cold sleep",
  "RTC Alarm Error occurred in cold sleep",
  "Unknown(13)",
  "Unknown(14)",
  "Unknown(15)",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "SEN_INT signal detected in cold sleep",
  "PMIC signal detected in cold sleep",
  "USB Disconnected in cold sleep",
  "USB Connected in cold sleep",
  "Power On Reset",
};

void printBootCause(bootcause_e bc)
{
  Serial.println("--------------------------------------------------");
  Serial.print("Boot Cause: ");
  Serial.print(boot_cause_strings[bc]);
  if ((COLD_GPIO_IRQ36 <= bc) && (bc <= COLD_GPIO_IRQ47)) {
    // Wakeup by GPIO
    int pin = LowPower.getWakeupPin(bc);
    Serial.print(" <- pin ");
    Serial.print(pin);
  }
  Serial.println();
  Serial.println("--------------------------------------------------");
}

// Pin used to trigger a wakeup
const uint8_t button2 = PIN_D02;
const uint8_t button3 = PIN_D03;

void pushed2()
{
  Serial.println("Pushed D02!");
}

void pushed3()
{
  Serial.println("Pushed D03!");
}

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  // Initialize LowPower library
  LowPower.begin();

  // Get the boot cause
  bootcause_e bc = LowPower.bootCause();

  if ((bc == POR_SUPPLY) || (bc == POR_NORMAL)) {
    Serial.println("Example for GPIO wakeup from cold sleep");
  } else {
    Serial.println("wakeup from cold sleep");
  }

  // Print the boot cause
  printBootCause(bc);

  // Print the current clock
  RTC.begin();
  RtcTime now = RTC.getTime();
  printf("%04d/%02d/%02d %02d:%02d:%02d\n",
         now.year(), now.month(), now.day(),
         now.hour(), now.minute(), now.second());

  // Button pin setting
  pinMode(button2, INPUT_PULLUP);
  attachInterrupt(button2, pushed2, FALLING);

  pinMode(button3, INPUT_PULLUP);
  attachInterrupt(button3, pushed3, FALLING);

  delay(5000);

  // Enable wakeup by pushing button2
  LowPower.enableBootCause(button2);
  // Disable wakeup by pushing button3
  LowPower.disableBootCause(button3);
  Serial.println("Wait until PIN_D02 is low.");

  // Cold sleep
  Serial.print("Go to cold sleep...");
  delay(1000);
  LowPower.coldSleep();
}

void loop()
{
}

