/*
 *  Storage.cpp - Spresense Arduino Storage library
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
 * @file Storage.cpp
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino Storage library
 * 
 * @details The Storage library allows for creating and removing files and directories
 *          on the storage, like the flash or SD card. The file operations such as
 *          writing and reading are performed via File library.
 */

#include <sdk/config.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#include <Arduino.h>
#include <Storage.h>

char *StorageClass::realpath(char *dest, const char *src, size_t n)
{
  strncpy(dest, mountdir, n);
  strncat(dest, src, strlen(src));
  return dest;
}

File StorageClass::open(const char *filepath, uint8_t mode)
{
  char fullpath[64];
  realpath(fullpath, filepath, sizeof(fullpath));

  return File(fullpath, mode);
}

boolean StorageClass::exists(const char *filepath)
{
  struct stat stat;
  char fullpath[64];

  if (filepath) {
    realpath(fullpath, filepath, sizeof(fullpath));
    return (::stat(fullpath, &stat) == 0);
  } else {
    return false;
  }
}

boolean StorageClass::mkdir(const char *filepath)
{
  struct stat stat;
  char *p;
  char tmp;
  char fullpath[64];

  if (!filepath)
    return false;

  realpath(fullpath, filepath, sizeof(fullpath));

  // create directories recursively
  for (p = &fullpath[1]; *p; ++p) {
    if (*p == '/' || *(p+1) == 0) {
      tmp = *p;
      *p = 0;
      if (::stat(fullpath, &stat) != 0 || !S_ISDIR(stat.st_mode)) {
        if (::mkdir(fullpath, 0777) != 0) {
          return false;
        }
      }
      *p = tmp;
    }
  }
  return true;
}

boolean StorageClass::rmdir(const char *filepath)
{
  char fullpath[64];

  if (!filepath)
    return false;

  realpath(fullpath, filepath, sizeof(fullpath));

  // remove the final letter '/'
  size_t n = strlen(fullpath);
  if (fullpath[n - 1] == '/') {
    fullpath[n - 1] = '\0';
  }
  return (::rmdir(fullpath) == 0);
}

boolean StorageClass::remove(const char *filepath)
{
  char fullpath[64];

  if (!filepath)
    return false;

  realpath(fullpath, filepath, sizeof(fullpath));
  return (::unlink(fullpath) == 0);
}

StorageClass Storage;
