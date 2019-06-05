/*
 *  Flash.h - Spresense Arduino Flash library
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

#ifndef __FLASH_H__
#define __FLASH_H__

#ifdef SUBCORE
#error "Flash library is NOT supported by SubCore."
#endif

/**
 * @defgroup flash Flash Library API
 * @brief API for using SPI-Flash
 * @{
 */

/**
 * @file Flash.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino Flash library
 * 
 * @details The Flash library allows for creating and removing files and directories
 *          on the flash. This is derivatived from Storage library. The file
 *          operations such as writing and reading are performed via File library.
 */

#include <sdk/config.h>

#include <Arduino.h>
#include <Storage.h>

/**
 * @class FlashClass
 * @brief The Flash class provides functions for accessing the flash and
 *        manipulating its files and directories.  
 */
class FlashClass : public StorageClass {

public:
  FlashClass();

  /**
   * @brief  Initialize the Flash library
   *
   * @details This is a dummy function provided to match other storages as
   *          SDHCI and eMMC.
   * @return true
   */
  boolean begin() { return true; };

  /**
   * @brief  Format the Flash device
   *
   * @details This will format the Flash device to SmartFS file system.
   * @return 0 if the Flash format is successful, error code if not
   */
  int format();
};

extern FlashClass Flash;

/** @} flash */

#endif
