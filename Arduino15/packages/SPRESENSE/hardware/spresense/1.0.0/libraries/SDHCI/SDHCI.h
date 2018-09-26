/*
 *  SDHCI.h - Spresense Arduino SDHCI library
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

#ifndef __SD_H__
#define __SD_H__

/**
 * @file SDHCI.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief SPRESENSE Arduino SDHCI library
 * 
 * @details The SDHCI library allows for reading from and writing to SD cards
 */

#include <Arduino.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#define FILE_READ O_RDONLY                                     /**< Open the file for reading, starting at the beginning of the file. */
#define FILE_WRITE (O_RDONLY | O_WRONLY | O_CREAT | O_APPEND)  /**< Open the file for reading and writing, starting at the end of the file.  */

#include <File.h>

namespace SDHCILib {


/**
 * @class SDClass
 * @brief The SD class provides functions for accessing the SD card and 
 *        manipulating its files and directories.  
 */
class SDClass {

public:
  SDClass() : mshandle(NULL) {};

  /**
  * @brief Opens a file on the SD card.
  * 
  * @details If the file is opened for writing, it will be created if it 
  *          doesn't already exist (but the directory containing it must 
  *          already exist). 
  * @param [in] filename The name of the file to open.
  * @param [in] mode The mode in which to open the file.
  * @return File object referring to the opened file.
  */
  File open(const char *filename, uint8_t mode = FILE_READ);

 /**
  * @brief Opens a file on the SD card.
  * 
  * @details If the file is opened for writing, it will be created if it 
  *          doesn't already exist (but the directory containing it must 
  *          already exist). 
  * @param [in] filename The name of the file to open.
  * @param [in] mode The mode in which to open the file.
  * @return File object referring to the opened file.
  */
  File open(const String &filename, uint8_t mode = FILE_READ) { return open( filename.c_str(), mode ); }

 /**
  * @brief Tests whether a file or directory exists on the SD card. 
  * 
  * @param [in] filepath The name of the file to test for existence.
  * @return true if the file or directory exists, false if not
  */
  boolean exists(const char *filepath);

 /**
  * @brief Tests whether a file or directory exists on the SD card. 
  * 
  * @param [in] filepath The name of the file to test for existence.
  * @return true if the file or directory exists, false if not
  */
  boolean exists(const String &filepath) { return exists(filepath.c_str()); }

 /**
  * @brief Create a directory on the SD card. 
  * 
  * @details This will also create any intermediate directories that don't already exists.
  * @param [in] filepath The name of the directory to create.
  * @return true if the creation of the directory succeeded, false if not 
  */
  boolean mkdir(const char *filepath);

 /**
  * @brief Create a directory on the SD card. 
  * 
  * @details This will also create any intermediate directories that don't already exists.
  * @param [in] filepath The name of the directory to create.
  * @return true if the creation of the directory succeeded, false if not 
  */
  boolean mkdir(const String &filepath) { return mkdir(filepath.c_str()); }

 /**
  * @brief Remove a file from the SD card.
  * 
  * @param [in] filepath The name of the file to remove.
  * @return true if the removal of the file succeeded, false if not
  */
  boolean remove(const char *filepath);

 /**
  * @brief Remove a file from the SD card.
  * 
  * @param [in] filepath The name of the file to remove.
  * @return true if the removal of the file succeeded, false if not
  */
  boolean remove(const String &filepath) { return remove(filepath.c_str()); }

 /**
  * @brief Remove a directory from the SD card.
  * 
  * @details The directory must be empty.
  * @param [in] filepath The name of the directory to remove.
  * @return true if the removal of the directory succeeded, false if not
  */
  boolean rmdir(const char *filepath);

 /**
  * @brief Remove a directory from the SD card.
  * 
  * @details The directory must be empty.
  * @param [in] filepath The name of the directory to remove.
  * @return true if the removal of the directory succeeded, false if not
  */
  boolean rmdir(const String &filepath) { return rmdir(filepath.c_str()); }

  /**
   * @brief Start USB Mass Storage Class
   */
  int beginUsbMsc();

  /**
   * @brief Stop USB Mass Storage Class
   */
  int endUsbMsc();

private:

  void *mshandle;
  friend class File;
};

};

/* This ensure compatibility with sketches that uses only SD library */
using namespace SDHCILib;

#endif
