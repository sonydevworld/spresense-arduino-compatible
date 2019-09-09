/*
 *  Storage.h - Spresense Arduino Storage library
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

#ifndef __STORAGE_H__
#define __STORAGE_H__

#ifdef SUBCORE
#error "Storage library is NOT supported by SubCore."
#endif

/**
 * @defgroup storage Storage Library API
 * @brief API for using storage
 * @{
 */

/**
 * @file Storage.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino Storage library
 * 
 * @details The Storage library allows for creating and removing files and directories
 *          on the storage, like the flash or SD card. The file operations such as
 *          writing and reading are performed via File library.
 */

#include <sdk/config.h>

#include <Arduino.h>
#include <File.h>

/**
 * @class StorageClass
 * @brief The Storage class provides functions for accessing the storage
 *        manipulating its files and directories.  
 */
class StorageClass {

private:
  boolean _realpath(char *dest, const char *src, size_t n);

protected:
  char mountdir[16];

public:
  StorageClass() : mountdir("") {};
  StorageClass(const char *str) { strncpy(mountdir, str, 16); };

  /**
   * @brief Opens a file on the Flash.
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
   * @brief Opens a file on the Flash.
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
   * @brief Tests whether a file or directory exists on the Flash.
   *
   * @param [in] filepath The name of the file to test for existence.
   * @return true if the file or directory exists, false if not
   */
  boolean exists(const char *filepath);

  /**
   * @brief Tests whether a file or directory exists on the Flash.
   *
   * @param [in] filepath The name of the file to test for existence.
   * @return true if the file or directory exists, false if not
   */
  boolean exists(const String &filepath) { return exists(filepath.c_str()); }

  /**
   * @brief Create a directory on the Flash.
   *
   * @details This will also create any intermediate directories that don't already exists.
   * @param [in] filepath The name of the directory to create.
   * @return true if the creation of the directory succeeded, false if not
   */
  boolean mkdir(const char *filepath);

  /**
   * @brief Create a directory on the Flash.
   *
   * @details This will also create any intermediate directories that don't already exists.
   * @param [in] filepath The name of the directory to create.
   * @return true if the creation of the directory succeeded, false if not
   */
  boolean mkdir(const String &filepath) { return mkdir(filepath.c_str()); }

  /**
   * @brief Remove a file from the Flash.
   *
   * @param [in] filepath The name of the file to remove.
   * @return true if the removal of the file succeeded, false if not
   */
  boolean remove(const char *filepath);

  /**
   * @brief Remove a file from the Flash.
   *
   * @param [in] filepath The name of the file to remove.
   * @return true if the removal of the file succeeded, false if not
   */
  boolean remove(const String &filepath) { return remove(filepath.c_str()); }

  /**
   * @brief Remove a directory from the Flash.
   *
   * @details The directory must be empty.
   * @param [in] filepath The name of the directory to remove.
   * @return true if the removal of the directory succeeded, false if not
   */
  boolean rmdir(const char *filepath);

  /**
   * @brief Remove a directory from the Flash.
   *
   * @details The directory must be empty.
   * @param [in] filepath The name of the directory to remove.
   * @return true if the removal of the directory succeeded, false if not
   */
  boolean rmdir(const String &filepath) { return rmdir(filepath.c_str()); }

};

extern StorageClass Storage;

/** @} storage */

#endif
