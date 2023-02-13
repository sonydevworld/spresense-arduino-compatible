/*
 *  eMMC.cpp - Spresense Arduino eMMC library
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
 * @file eMMC.cpp
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino eMMC library
 * 
 * @details The eMMC library allows for creating and removing files and directories
 *          on the eMMC. This is derivatived from Storage library. The file
 *          operations such as writing and reading are performed via File library.
 */

#include <sdk/config.h>

#include <Arduino.h>
#include <eMMC.h>

#include <sys/boardctl.h>
#include <nuttx/usb/usbdev.h>
#include <fsutils/mkfatfs.h>
#include <arch/board/board.h>

#ifndef CONFIG_SYSTEM_USBMSC_NLUNS
#  define CONFIG_SYSTEM_USBMSC_NLUNS 1
#endif

#define EMMC_DEVPATH "/dev/emmc0"

#define EMMC_MOUNT_POINT "/mnt/emmc/"

#define EMMC_POWER_PIN_UNKNOWN 0xff

//#define DEBUG
#ifdef DEBUG
#  define DebugPrintf(fmt, ...) printf(fmt, ## __VA_ARGS__)
#else
#  define DebugPrintf(fmt, ...) ((void)0)
#endif

eMMCClass::eMMCClass() : StorageClass(EMMC_MOUNT_POINT), mshandle(NULL), power_pin(EMMC_POWER_PIN_UNKNOWN)
{
}

boolean eMMCClass::begin(uint8_t pin)
{
  power_pin = pin;
  
  pinMode(power_pin, OUTPUT);
  digitalWrite(power_pin, HIGH);

  /* device bootup time */
  delay(5);

  return begin();
}

boolean eMMCClass::begin()
{
  int ret;

  /* Initialize and mount the eMMC device */
  ret = board_emmc_initialize();
  if (ret != 0)
    {
      return false;
    }

  return true;
}

boolean eMMCClass::end()
{
  int ret = board_emmc_finalize();
  if (ret != 0)
    {
      return false;
    }

  /* Finalize and unmount the eMMC device */
  if(power_pin != EMMC_POWER_PIN_UNKNOWN)
    {
      digitalWrite(power_pin, LOW);
      power_pin = 0xff;
    }

  return true;
}

int eMMCClass::beginUsbMsc()
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

  ret = usbmsc_bindlun(handle, EMMC_DEVPATH, 0, 0, 0, false);
  if (ret < 0)
    {
      DebugPrintf("usbmsc_bindlun failed for LUN 1 using %s: %d\n",
                  EMMC_DEVPATH, -ret);
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

int eMMCClass::endUsbMsc()
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

int eMMCClass::format(uint8_t fattype)
{
  int ret;
  struct fat_format_s fmt = FAT_FORMAT_INITIALIZER;

  /* FAT size: 0 (autoselect), 12, 16, or 32 */
  if ((fattype != 0) && (fattype != 12) &&
      (fattype != 16) && (fattype != 32)) {
    return -1;
  }

  fmt.ff_fattype = fattype;

  ret = mkfatfs(EMMC_DEVPATH, &fmt);
  return ret;
}

eMMCClass eMMC;
