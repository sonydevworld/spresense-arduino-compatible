/*
 *  SDHCI.h - Spresense Arduino SDHCI library
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

#ifndef __SD_H__
#define __SD_H__

#ifdef SUBCORE
#error "SDHCI library is NOT supported by SubCore."
#endif

/**
 * @defgroup sdhci SD-card Library API
 * @brief API for using SD Card
 * @{
 */

/**
 * @file SDHCI.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino SDHCI library
 * 
 * @details The SDHCI library allows for creating and removing files and directories
 *          on the SD card. This is derivatived from Storage library. The file
 *          operations such as writing and reading are performed via File library.
 */

#include <sdk/config.h>

#include <Arduino.h>
#include <Storage.h>

/**
 * @class SDClass
 * @brief The SD class provides functions for accessing the SD card and 
 *        manipulating its files and directories. Also, this class also provides 
 *        the USB Mass storage function.
 */
class SDClass : public StorageClass {

public:
  SDClass();

  /**
   * @brief  Initialize the SD library
   *
   * @details This will check that the SD card is inserted and mounted or not.
   *          This needs to be called to set up the connection to the SD card
   *          before other methods are used.
   * @param [in] dummy dummy argument to keep compatibility with Arduino SD library
   * @return true if the SD card is inserted and mounted, false if not
   */
  boolean begin(uint8_t dummy = 0);

  /**
   * @brief Start USB Mass Storage Class
   */
  int beginUsbMsc();

  /**
   * @brief Stop USB Mass Storage Class
   */
  int endUsbMsc();

  /**
   * @brief  Format the SD card
   *
   * @details This will format the unformatted SD card to FAT file system.
   * @param [in] fattype FAT type: 12 or 16 or 32 (default 32)
   * @return 0 if the SD card format is successful, error code if not
   */
  int format(uint8_t fattype = 32);

private:
  void *mshandle;
};

/** @} sdhci */

#endif
