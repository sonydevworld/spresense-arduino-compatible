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
void WatchdogClass::begin(void)
{
  if (0 <= wd_fd)
    {
      watchdog_printf("watchdog: %s already opened\n",
             WATCHDOG_DEVPATH);
      return;
    }

  wd_fd = open(WATCHDOG_DEVPATH, O_RDONLY);
  if (wd_fd < 0)
    {
      watchdog_printf("watchdog: open %s failed\n",
             WATCHDOG_DEVPATH);
    }
}

// Public : Start the Watchdog
void WatchdogClass::start(uint32_t timeout) {
  int ret;

  if (0 <= wd_fd)
    {
      ret = ioctl(wd_fd, WDIOC_SETTIMEOUT, (unsigned long)timeout);
      if (ret < 0)
        {
          watchdog_printf("watchdog: ioctl(WDIOC_SETTIMEOUT) failed\n");
          return;
        }

      ret = ioctl(wd_fd, WDIOC_START, 0);
      if (ret < 0)
        {
          watchdog_printf("watchdog: ioctl(WDIOC_START) failed\n");
        }
    }
  else
    {
      watchdog_printf("watchdog: watchdog not initialized.\n");
    }
}

// Public : Send kick to avoid bite
void WatchdogClass::kick(void) {
  int ret;

  if (0 <= wd_fd)
    {
      ret = ioctl(wd_fd, WDIOC_KEEPALIVE, 0);
      if (ret < 0)
        {
          watchdog_printf("watchdog: ioctl(WDIOC_KEEPALIVE) failed\n");
        }
    }
  else
    {
      watchdog_printf("watchdog: watchdog not initialized.\n");
    }
}

// Public : Get remain time for bite a watchdog
uint32_t WatchdogClass::timeleft(void) {
  struct watchdog_status_s status;
  int ret;

  if (0 <= wd_fd)
    {
      ret = ioctl(wd_fd, WDIOC_GETSTATUS, (unsigned long)&status);
      if (ret < 0)
        {
          watchdog_printf("watchdog: ioctl(WDIOC_GETSTATUS) failed\n");
        }

      return status.timeleft;
    }
  else
    {
      watchdog_printf("watchdog: watchdog not initialized.\n");
      return -1;
    }
}

// Public : Stop the Watchdog
void WatchdogClass::stop(void) {
  int ret;

  if (0 <= wd_fd)
    {
      ret = ioctl(wd_fd, WDIOC_STOP, 0);
      if (ret < 0)
        {
          watchdog_printf("watchdog: ioctl(WDIOC_STOP) failed\n");
        }
    }
  else
    {
      watchdog_printf("watchdog: watchdog not initialized.\n");
    }
}

// Public : Finalize to use the Watchdog
void WatchdogClass::end(void)
{
  if (0 <= wd_fd)
    {
      stop();
      close(wd_fd);
      wd_fd = -1;
    }
}

