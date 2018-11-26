/*
 *  LowPower.cpp - Spresense Arduino Low Power library
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

#include <sdk/config.h>

#include <unistd.h>
#include <nuttx/irq.h>
#include <sys/boardctl.h>
#include <arch/chip/pm.h>
#include <cxd56_gpioint.h>

#include <RTC.h>
#include "LowPower.h"
#include "wiring_private.h"

void LowPowerClass::begin()
{
  RTC.begin();
}

void LowPowerClass::end()
{
  ;
}

void LowPowerClass::sleep(uint32_t seconds)
{
  sleep(seconds);
}

void LowPowerClass::coldSleep()
{
  boardctl(BOARDIOC_POWEROFF, 1);
}

void LowPowerClass::coldSleep(uint32_t seconds)
{
  RTC.setAlarmSeconds(seconds);
  coldSleep();
}

void LowPowerClass::deepSleep()
{
  boardctl(BOARDIOC_POWEROFF, 0);
}

void LowPowerClass::deepSleep(uint32_t seconds)
{
  RTC.setAlarmSeconds(seconds);
  deepSleep();
}

void LowPowerClass::reboot()
{
  boardctl(BOARDIOC_RESET, 0);
}

bootcause_e LowPowerClass::bootCause()
{
  bootcause_e bc = POR_NORMAL;
  uint32_t bootcause = up_pm_get_bootcause();
  int i;

  for (i = 0; i < POR_NORMAL; i++) {
    if (bootcause & (1 << i)) {
      bc = (bootcause_e)i;
      break;
    }
  }
  return bc;
}

bool LowPowerClass::isEnabledBootCause(bootcause_e bc)
{
  uint32_t bootmask = up_pm_get_bootmask();

  if (POR_NORMAL <= bc) {
    return false;
  }

  if (bootmask & (1 << bc)) {
    return true;
  } else {
    return false;
  }
}

void LowPowerClass::enableBootCause(bootcause_e bc)
{
  if (bc < POR_NORMAL) {
    up_pm_set_bootmask(1 << bc);
  }
}

void LowPowerClass::disableBootCause(bootcause_e bc)
{
  if (bc < POR_NORMAL) {
    up_pm_clr_bootmask(1 << bc);
  }
}

uint8_t LowPowerClass::getWakeupPin(bootcause_e bc)
{
  uint8_t pin = 255;

  if ((COLD_GPIO_IRQ36 <= bc) && (bc <= COLD_GPIO_IRQ47)) {
    int irq = bc - COLD_GPIO_IRQ36 + CXD56_IRQ_EXDEVICE_0;
    int _pin = cxd56_gpioint_pin(irq);
    pin = pin_invert(_pin);
  }

  return pin;
}

LowPowerClass LowPower;

