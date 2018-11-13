/*
 *  Watchdog.cpp - Spresense Arduino Watchdog library 
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

/**
 * @file Watchdog.cpp
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino Watchdog library 
 * 
 * @details It is a library for using watchdog for user program, and
 *          user can reset if program is freezed.
 */

#include <sdk/config.h>

#include <fcntl.h>
#include <sys/ioctl.h>

#include "Watchdog.h"

#define WATCHDOG_DEVPATH "/dev/watchdog0"

// Public instance
WatchdogClass Watchdog;

// Public : Constructor
WatchdogClass::WatchdogClass(void)
:wd_fd(-1)
{
}

// Public : Initialize to use the Watchdog
void WatchdogClass::begin(uint32_t timeout)
{
  int ret;
  
  wd_fd = open(WATCHDOG_DEVPATH, O_RDONLY);
  if (wd_fd < 0)
    {
      printf("watchdog: open %s failed\n",
             WATCHDOG_DEVPATH);
    }

  ret = ioctl(wd_fd, WDIOC_SETTIMEOUT, (unsigned long)timeout);
  if (ret < 0)
    {
      printf("watchdog: ioctl(WDIOC_SETTIMEOUT) failed\n");
    }
}

// Public : Start the Watchdog
void WatchdogClass::start(void) {
  int ret;
  
  ret = ioctl(wd_fd, WDIOC_START, 0);
  if (ret < 0)
    {
      printf("wdog_main: ioctl(WDIOC_START) failed\n");
    }
}

// Public : Send kick to avoid bite
void WatchdogClass::kick(void) {
  int ret;
  
  ret = ioctl(wd_fd, WDIOC_KEEPALIVE, 0);
  if (ret < 0)
    {
      printf("wdog_main: ioctl(WDIOC_KEEPALIVE) failed\n");
    }
}

// Public : Stop the Watchdog
void WatchdogClass::stop(void) {
  int ret;
  
  ret = ioctl(wd_fd, WDIOC_STOP, 0);
  if (ret < 0)
    {
      printf("wdog_main: ioctl(WDIOC_STOP) failed\n");
    }
}

// Public : Finalize to use the Watchdog
void WatchdogClass::end(void)
{
    stop();
    close(wd_fd);
}

