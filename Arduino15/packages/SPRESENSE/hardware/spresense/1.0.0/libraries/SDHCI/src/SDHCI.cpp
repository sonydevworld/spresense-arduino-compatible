/*
 *  SDHCI.cpp - Spresense Arduino SDHCI library
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

/**
 * @file SDHCI.cpp
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino SDHCI library
 * 
 * @details The SDHCI library allows for creating and removing files and directories
 *          on the SD card. This is derivatived from Storage library. The file
 *          operations such as writing and reading are performed via File library.
 */

#include <sdk/config.h>

#include <Arduino.h>
#include <SDHCI.h>

#include <sys/boardctl.h>
#include <nuttx/usb/usbdev.h>
#include <fsutils/mkfatfs.h>

#ifndef CONFIG_SYSTEM_USBMSC_NLUNS
#  define CONFIG_SYSTEM_USBMSC_NLUNS 1
#endif

#define SD_DEVPATH "/dev/mmcsd0"

#define SD_MOUNT_POINT "/mnt/sd0/"

//#define DEBUG
#ifdef DEBUG
#  define DebugPrintf(fmt, ...) printf(fmt, ## __VA_ARGS__)
#else
#  define DebugPrintf(fmt, ...) ((void)0)
#endif

SDClass::SDClass() : StorageClass(SD_MOUNT_POINT), mshandle(NULL)
{
}

boolean SDClass::begin(uint8_t dummy)
{
  struct stat buf;
  int retry;
  int ret;

  (void)dummy;

  /* In case that SD card isn't inserted, it times out at max 2 sec */
  for (retry = 0; retry < 20; retry++) {
    ret = stat(SD_MOUNT_POINT, &buf);
    if (ret == 0) {
      return true;
    }
    usleep(100 * 1000); // 100 msec
  }
  return false;
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

  ret = usbmsc_bindlun(handle, SD_DEVPATH, 0, 0, 0, false);
  if (ret < 0)
    {
      DebugPrintf("usbmsc_bindlun failed for LUN 1 using %s: %d\n",
                  SD_DEVPATH, -ret);
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

int SDClass::format(uint8_t fattype)
{
  int ret;
  struct fat_format_s fmt = FAT_FORMAT_INITIALIZER;

  /* Default is FAT32, but it's possible to format at FAT12 or FAT16. */
  if ((fattype == 12) || (fattype == 16)) {
    fmt.ff_fattype = fattype;
  }

  ret = mkfatfs(SD_DEVPATH, &fmt);
  return ret;
}

//SDClass SD;
