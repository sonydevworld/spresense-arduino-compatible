/*
 *  Flash.cpp - Spresense Arduino Flash library
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
 * @file Flash.cpp
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino Flash library
 * 
 * @details The Flash library allows for creating and removing files and directories
 *          on the flash. This is derivatived from Storage library. The file
 *          operations such as writing and reading are performed via File library.
 */

#include <sdk/config.h>

#include <Arduino.h>
#include <Flash.h>

#define FLASH_DEVPATH "/dev/smart0d1"
#define FLASH_SECTORSIZE 4096
#define FLASH_NROOTDIRS 1

#define FLASH_MOUNT_POINT "/mnt/spif/"

extern "C" {
int mksmartfs(const char *pathname, uint16_t sectorsize, uint8_t nrootdirs);
};

FlashClass::FlashClass() : StorageClass(FLASH_MOUNT_POINT)
{
}

int FlashClass::format()
{
  return mksmartfs(FLASH_DEVPATH, FLASH_SECTORSIZE, FLASH_NROOTDIRS);
}

FlashClass Flash;
