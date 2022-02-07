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

#include <stdio.h>
#include <unistd.h>
#include <nuttx/irq.h>
#include <sys/boardctl.h>
#include <arch/chip/pm.h>
#include <cxd56_gpioint.h>
#include <cxd56_clock.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <nuttx/power/battery_charger.h>
#include <nuttx/power/battery_ioctl.h>

#include <arch/chip/battery_ioctl.h>
#include <arch/board/board.h>
#include <arch/chip/pm.h>

#include <RTC.h>
#include "LowPower.h"
#include "wiring_private.h"

#define DEV_BATT "/dev/bat"

#define ERRMSG(format, ...) printf("ERROR: " format, ##__VA_ARGS__)

void LowPowerClass::begin()
{
  if (isInitialized) {
    return;
  }

  RTC.begin();

  board_charger_initialize(DEV_BATT);

  isInitialized = true;
}

void LowPowerClass::end()
{
  board_charger_uninitialize(DEV_BATT);

  isInitialized = false;
}

void LowPowerClass::sleep(uint32_t seconds)
{
  ::sleep(seconds);
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

bool LowPowerClass::isEnabledBootCause(uint8_t pin)
{
  bootcause_e bc = pin2bootcause(pin);

  return isEnabledBootCause(bc);
}

void LowPowerClass::enableBootCause(bootcause_e bc)
{
  if (bc < POR_NORMAL) {
    up_pm_set_bootmask(1 << bc);
  }
}

void LowPowerClass::enableBootCause(uint8_t pin)
{
  bootcause_e bc = pin2bootcause(pin);

  enableBootCause(bc);
}

void LowPowerClass::disableBootCause(bootcause_e bc)
{
  if (bc < POR_NORMAL) {
    up_pm_clr_bootmask(1 << bc);
  }
}

void LowPowerClass::disableBootCause(uint8_t pin)
{
  bootcause_e bc = pin2bootcause(pin);

  disableBootCause(bc);
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

void LowPowerClass::clockMode(clockmode_e mode)
{
  int count;

  if (!isEnabledDVFS) {
    board_clock_enable();
    isEnabledDVFS = true;
  }

  switch (mode) {
  case CLOCK_MODE_156MHz:
    up_pm_acquire_freqlock(&hvlock);
    break;
  case CLOCK_MODE_32MHz:
    up_pm_acquire_freqlock(&lvlock);
    count = up_pm_get_freqlock_count(&hvlock);
    while (count--) {
      up_pm_release_freqlock(&hvlock);
    }
    break;
  case CLOCK_MODE_8MHz:
    count = up_pm_get_freqlock_count(&hvlock);
    while (count--) {
      up_pm_release_freqlock(&hvlock);
    }
    count = up_pm_get_freqlock_count(&lvlock);
    while (count--) {
      up_pm_release_freqlock(&lvlock);
    }
    break;
  default:
    break;
  }
}

clockmode_e LowPowerClass::getClockMode()
{
  uint32_t clock;

  clock = cxd56_get_cpu_baseclk();

  if (clock >= 100 * 1000 * 1000) { /* >= 100MHz */
    return CLOCK_MODE_156MHz;
  } else if (clock >= 16 * 1000 * 1000) { /* >= 16MHz */
    return CLOCK_MODE_32MHz;
  } else { /* < 16MHz */
    return CLOCK_MODE_8MHz;
  }
}

int LowPowerClass::getVoltage(void)
{
  int fd;
  int voltage;
  int ret;

  if (!isInitialized) {
    ERRMSG("ERROR: begin() not called\n");
    return 0;
  }

  fd = open(DEV_BATT, O_RDWR);
  if (fd < 0) {
    return fd;
  }
  ret = ioctl(fd, BATIOC_GET_VOLTAGE, (unsigned long)(uintptr_t)&voltage);
  if (ret < 0) {
    close(fd);
    return ret;
  }
  close(fd);

  return voltage;
}

int LowPowerClass::getCurrent(void)
{
  int fd;
  int current;
  int ret;

  if (!isInitialized) {
    ERRMSG("ERROR: begin() not called\n");
    return 0;
  }

  fd = open(DEV_BATT, O_RDWR);
  if (fd < 0) {
    return fd;
  }
  ret = ioctl(fd, BATIOC_GET_CURRENT, (unsigned long)(uintptr_t)&current);
  if (ret < 0) {
    close(fd);
    return ret;
  }
  close(fd);

  return current;
}

bootcause_e LowPowerClass::pin2bootcause(uint8_t pin)
{
  bootcause_e bc = POR_NORMAL;

  uint8_t _pin = pin_convert(pin);
  int irq = cxd56_gpioint_irq(_pin);

  if (irq > 0) {
    bc = (bootcause_e)(irq - CXD56_IRQ_EXDEVICE_0 + COLD_GPIO_IRQ36);
  }

  return bc;
}

LowPowerClass LowPower;

