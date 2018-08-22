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
#include <fcntl.h>
#include <stdio.h>

#define FILE_READ O_RDONLY                                     /**< Open the file for reading, starting at the beginning of the file. */
#define FILE_WRITE (O_RDONLY | O_WRONLY | O_CREAT | O_APPEND)  /**< Open the file for reading and writing, starting at the end of the file.  */

namespace SDHCILib {

/**
 * @class File
 * @brief The File class allows for reading from and writing to individual files 
 *        on the SD card. 
 */
class File : public Stream {

private:
  char* _name;            /**<  The name of the file. */
  FILE* _fd;              /**<  The pointer to a File object. */
  unsigned long _size;    /**<  The size of the file. */
  unsigned long _curpos;  /**<  The current position within the file. */
  void* _dir;             /**<  The pointer to the directory stream. */

public:
 /**
  * @brief Construct a new File object
  * 
  * @param [in] name The name of the file
  * @param [in] mode The mode of the file
  */
  File(const char *name, uint8_t mode = FILE_READ);

 /**
  * @brief Construct a new File object
  */
  File();

 /**
  * @brief Destroy File object
  */
  ~File();

 /**
  * @brief Write data to the file. 
  * 
  * @param [in] data The byte to write.
  * @return The number of bytes written.
  */
  virtual size_t write(uint8_t data);

 /**
  * @brief Write data to the file. 
  * 
  * @param [in] buf Array of bytes.
  * @param [in] size The number of elements in buf.
  * @return The number of bytes written.
  */
  virtual size_t write(const uint8_t *buf, size_t size);

 /**
  * @brief Read from the file. 
  * 
  * @return The next byte, or -1 if none is available. 
  */
  virtual int read();

 /**
  * @brief Read a byte from the file without advancing to the next one.
  * 
  * @return The next byte, or -1 if none is available. 
  */
  virtual int peek();

 /**
  * @brief Check if there are any bytes available for reading from the file. 
  * 
  * @return the number of bytes available 
  */
  virtual int available();

 /**
  * @brief Ensures that any bytes written to the file are physically saved to the SD card.
  * 
  * @details This is done automatically when the file is closed. 
  */
  virtual void flush();

 /**
  * @brief Read from the file. 
  * 
  * @param [out] buf Array of bytes.
  * @param [in] nbyte The number of elements in buf.
  * @return The total number of bytes successfully read, or -1 if none is available 
  */
  int read(void *buf, uint16_t nbyte);

 /**
  * @brief Seek to a new position in the file.
  * 
  * @param [in] pos The position to which to seek.
  * @return true for success, false for failure
  */
  boolean seek(uint32_t pos);

 /**
  * @brief Get the current position within the file. 
  * 
  * @return the position within the file 
  */
  uint32_t position();

 /**
  * @brief Get the size of the file. 
  * 
  * @return the size of the file in bytes
  */
  uint32_t size();

 /**
  * @brief Close the file.
  * 
  * @details Ensure that any data written to it is physically saved to the SD card. 
  */
  void close();

 /**
  * @brief Check if the file or directory exists. 
  */
  operator bool();

 /**
  * @brief Returns the file name.
  * 
  * @return the file name
  */
  char *name();

 /**
  * @brief Check if the current file is a directory or not. 
  * 
  * @return true if is a directory, false if not
  */
  boolean isDirectory(void);

 /**
  * @brief Reports the next file or folder in a directory.  
  * 
  * @param [in] mode The mode in which to open the next file.
  * @return The next file or folder in the path 
  */
  File openNextFile(uint8_t mode = O_RDONLY);

   /**
  * @brief Bring you back to the first file in the directory.
  */
  void rewindDirectory(void);

  using Print::write;
};

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
