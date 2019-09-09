/*
 *  MPMutex.h - Spresense Arduino Multi-Processer Mutex library
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

#ifndef _MPMUTEX_H_
#define _MPMUTEX_H_

/**
 * @file MPMutex.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino Multi-Processer Mutex library
 *
 * @details The MP library can manage the Multi-processor Mutex.
 */

/**
 * @defgroup mpmutex MP Mutex Library API
 * @brief MP Mutex API
 * @{
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <Arduino.h>
#include <sdk/config.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <cxd56_sph.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define MP_MUTEX_ID0  "/dev/hsem14"
#define MP_MUTEX_ID1  "/dev/hsem13"
#define MP_MUTEX_ID2  "/dev/hsem12"
#define MP_MUTEX_ID3  "/dev/hsem11"
#define MP_MUTEX_ID4  "/dev/hsem10"
#define MP_MUTEX_ID5  "/dev/hsem9"
#define MP_MUTEX_ID6  "/dev/hsem8"
#define MP_MUTEX_ID7  "/dev/hsem7"
#define MP_MUTEX_ID8  "/dev/hsem6"
#define MP_MUTEX_ID9  "/dev/hsem5"
#define MP_MUTEX_ID10 "/dev/hsem4"

/****************************************************************************
 * class declaration
 ****************************************************************************/

/**
 * @class MPMutex
 * @brief This is the interface for MP Mutex.
 *
 */
class MPMutex
{
public:
  MPMutex(const char *devname) : _fd(-1) {
    strncpy(_devname, devname, sizeof(_devname));
  };
  ~MPMutex() {
    if (_fd >= 0) {
      close(_fd);
    }
  }
#if 0 /* not support yet */
  int Lock() {
    if (_create()) { return -1; }
    return ioctl(_fd, HSLOCK, 0);
  };
#endif
  int Trylock() {
    if (_create()) { return -1; }
    return ioctl(_fd, HSTRYLOCK, 0);
  };
  int Unlock() {
    if (_create()) { return -1; }
    return ioctl(_fd, HSUNLOCK, 0);
  };

private:
  int  _fd;
  char _devname[16];
  int  _create() {
    if (_fd < 0) {
      _fd = open(_devname, 0);
      return (_fd < 0) ? -1 : 0;
    }
    return 0;
  }
};

/** @} mpmutex */

#endif /* _MPMUTEX_H_ */
