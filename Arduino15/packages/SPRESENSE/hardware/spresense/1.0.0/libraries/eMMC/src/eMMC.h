/*
 *  eMMC.h - Spresense Arduino eMMC library
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

#ifndef __eMMC_H__
#define __eMMC_H__

#ifdef SUBCORE
#error "eMMC library is NOT supported by SubCore."
#endif

/**
 * @defgroup emmc eMMC Library API
 * @brief API for using eMMC
 * @{
 */

/**
 * @file eMMC.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino eMMC library
 * 
 * @details The eMMC library allows for creating and removing files and directories
 *          on the eMMC. This is derivatived from Storage library. The file
 *          operations such as writing and reading are performed via File library.
 */

#include <sdk/config.h>

#include <Arduino.h>
#include <Storage.h>

/**
 * @class eMMCClass
 * @brief The eMMC class provides functions for accessing the eMMC and 
 *        manipulating its files and directories. Also, this class also provides 
 *        the USB Mass storage function.
 */
class eMMCClass : public StorageClass {

public:
  eMMCClass();

  /**
   * @brief  Initialize the eMMC library
   *
   * @details This will check that the eMMC is mounted or not after being initialized it.
   *          This needs to be called to set up the connection to the eMMC
   *          before other methods are used.
   * @return true if the eMMC is mounted, false if not
   */
  boolean begin();

  /**
   * @brief  Initialize the eMMC library with Power on PIN
   *
   * @details This will check that the eMMC is mounted or not after being initialized it.
   *          This needs to be called to set up the connection to the eMMC
   *          before other methods are used.
   * @param [in] Power control pin for eMMC
   * @return true if the eMMC is mounted, false if not
   */
  boolean begin(uint8_t);

  /**
   * @brief  Finalize the eMMC library
   *
   * @details This function unmount, finalize device and power off device.
   * @return true if the eMMC finalization complete, false if not
   */
  boolean end();

  /**
   * @brief Start USB Mass Storage Class
   */
  int beginUsbMsc();

  /**
   * @brief Stop USB Mass Storage Class
   */
  int endUsbMsc();

  /**
   * @brief  Format the eMMC device
   *
   * @details This will format the unformatted eMMC device to FAT file system.
   * @param [in] fattype FAT type: 12 or 16 or 32 (default 32)
   * @return 0 if the eMMC format is successful, error code if not
   */
  int format(uint8_t fattype = 32);

private:
  void *mshandle;
  uint8_t power_pin;
};

extern eMMCClass eMMC;

/** @} emmc */

#endif
