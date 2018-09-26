/*
 *  SDHCI.cpp - Spresense Arduino SDHCI library
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
 * @file SDHCI.cpp
 * @author Sony Semiconductor Solutions Corporation
 * @brief SPRESENSE Arduino SDHCI library
 * 
 * @details The SDHCI library allows for reading from and writing to SD cards
 */

#include <sdk/config.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include "SDHCI.h"

#include <sys/boardctl.h>
#include <nuttx/usb/usbdev.h>

#ifndef CONFIG_SYSTEM_USBMSC_NLUNS
#  define CONFIG_SYSTEM_USBMSC_NLUNS 1
#endif

#ifndef CONFIG_SYSTEM_USBMSC_DEVPATH1
#  define CONFIG_SYSTEM_USBMSC_DEVPATH1 "/dev/mmcsd0"
#endif

//#define DEBUG
#ifdef DEBUG
#  define DebugPrintf(fmt, ...) printf(fmt, ## __VA_ARGS__)
#else
#  define DebugPrintf(fmt, ...) ((void)0)
#endif

#define STDIO_BUFFER_SIZE     4096           /**< STDIO buffer size. */
#define MAX_PATH_LENGTH        128           /**< Max path lenght. */       
static char SD_MOUNT_POINT[] = "/mnt/sd0/";  /**< SD mount point. */

namespace SDHCILib {

/**
 * @brief Creates the full path name for the specified relative path name.
 * 
 * @param [out] buf The buffer for full path name.
 * @param [in] bufsize Size of buffer for full path name.
 * @param [in] filepath Relative file path name.
 * @return full path name
 */
static char* fullpathname(char* buf, int bufsize, const char * filepath)
{
  if ((strlen(filepath) + sizeof(SD_MOUNT_POINT) <= (size_t)bufsize) && (bufsize >= 0)) {
    strcpy(buf, SD_MOUNT_POINT);
    strcat(buf, filepath);
    return buf;
  }

  return NULL;
}

File SDClass::open(const char *filepath, uint8_t mode) {
  return File(filepath, mode);
}

boolean SDClass::exists(const char *filepath) {
  struct stat stat;
  char fpbuf[MAX_PATH_LENGTH];
  char *fpname = fullpathname(fpbuf, MAX_PATH_LENGTH, filepath);

  if (fpname) {
    return (::stat(fpname, &stat) == 0);
  } else {
    return false;
  }
}

boolean SDClass::mkdir(const char *filepath) {
  struct stat stat;
  char fpbuf[MAX_PATH_LENGTH];
  char *fpname = fullpathname(fpbuf, MAX_PATH_LENGTH, filepath);
  char *p;
  char tmp;

  if (!fpname)
    return false;

  // create directories recursively
  for (p = fpname + sizeof(SD_MOUNT_POINT); *p; ++p) {
    if (*p == '/' || *(p+1) == 0) {
      tmp = *p;
      *p = 0;
      if (::stat(fpname, &stat) != 0 || !S_ISDIR(stat.st_mode)) {
        if (::mkdir(fpname, 0777) != 0) {
            return false;
        }
        *p = tmp;
      }
    }
  }

  return true;
}

boolean SDClass::rmdir(const char *filepath) {
  char fpbuf[MAX_PATH_LENGTH];
  char *fpname = fullpathname(fpbuf, MAX_PATH_LENGTH, filepath);

  if (!fpname)
    return false;

  return (::rmdir(fpname) == 0);
}

boolean SDClass::remove(const char *filepath) {
  char fpbuf[MAX_PATH_LENGTH];
  char *fpname = fullpathname(fpbuf, MAX_PATH_LENGTH, filepath);

  if (!fpname)
    return false;

  return (::unlink(fpname) == 0);
}

int SDClass::beginUsbMsc()
{
  struct boardioc_usbdev_ctrl_s ctrl;
  FAR void *handle;
  int ret;

  /* Check if there is a non-NULL USB mass storage device handle (meaning that the
   * USB mass storage device is already configured).
   */

  if (mshandle)
    {
      DebugPrintf("ERROR: Already connected\n");
      return 0;
    }

  /* Register block drivers (architecture-specific) */

  ctrl.usbdev   = BOARDIOC_USBDEV_MSC;
  ctrl.action   = BOARDIOC_USBDEV_INITIALIZE;
  ctrl.instance = 0;
  ctrl.handle   = NULL;

  ret = boardctl(BOARDIOC_USBDEV_CONTROL, (uintptr_t)&ctrl);
  if (ret < 0)
    {
      DebugPrintf("boardctl(BOARDIOC_USBDEV_CONTROL) failed: %d\n", -ret);
      return -1;
    }

  /* Then exports the LUN(s) */

  ret = usbmsc_configure(CONFIG_SYSTEM_USBMSC_NLUNS, &handle);
  if (ret < 0)
    {
      DebugPrintf("usbmsc_configure failed: %d\n", -ret);
      goto failure;
    }

  ret = usbmsc_bindlun(handle, CONFIG_SYSTEM_USBMSC_DEVPATH1, 0, 0, 0, false);
  if (ret < 0)
    {
      DebugPrintf("usbmsc_bindlun failed for LUN 1 using %s: %d\n",
                  CONFIG_SYSTEM_USBMSC_DEVPATH1, -ret);
      goto failure;
    }

#if !defined(CONFIG_USBDEV_COMPOSITE) || !defined(CONFIG_USBMSC_COMPOSITE)
  ret = usbmsc_exportluns(handle);
  if (ret < 0)
    {
      DebugPrintf("usbmsc_exportluns failed: %d\n", -ret);
      goto failure;
    }
#endif

  mshandle = handle;

  return 0;

failure:
  ctrl.usbdev   = BOARDIOC_USBDEV_MSC;
  ctrl.action   = BOARDIOC_USBDEV_DISCONNECT;
  ctrl.instance = 0;
  ctrl.handle   = &handle;

  (void)boardctl(BOARDIOC_USBDEV_CONTROL, (uintptr_t)&ctrl);
  return -1;
}

int SDClass::endUsbMsc()
{
  struct boardioc_usbdev_ctrl_s ctrl;

  /* First check if the USB mass storage device is already connected */

  if (!mshandle)
    {
      DebugPrintf("ERROR: Not connected\n");
      return -1;
    }

  /* Then disconnect the device and uninitialize the USB mass storage driver */

  ctrl.usbdev   = BOARDIOC_USBDEV_MSC;
  ctrl.action   = BOARDIOC_USBDEV_DISCONNECT;
  ctrl.instance = 0;
  ctrl.handle   = &mshandle;

  (void)boardctl(BOARDIOC_USBDEV_CONTROL, (uintptr_t)&ctrl);

  mshandle = NULL;

  return 0;
}

};
