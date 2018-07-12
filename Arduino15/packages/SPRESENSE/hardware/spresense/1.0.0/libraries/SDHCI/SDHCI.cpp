/*
 *  SDHCI.cpp - Spresense Arduino SDHCI library
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
 * @file SDHCI.cpp
 * @author Sony Corporation
 * @brief SPRESENSE Arduino SDHCI library
 * 
 * @details The SDHCI library allows for reading from and writing to SD cards
 */

#include <sdk/config.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include "SDHCI.h"

#include <sys/boardctl.h>
#include <nuttx/usb/usbdev.h>

#ifndef CONFIG_SYSTEM_USBMSC_NLUNS
#  define CONFIG_SYSTEM_USBMSC_NLUNS 1
#endif

#ifndef CONFIG_SYSTEM_USBMSC_DEVPATH1
#  define CONFIG_SYSTEM_USBMSC_DEVPATH1 "/dev/mmcsd0"
#endif

//#define DEBUG
#ifdef DEBUG
#  define DebugPrintf(fmt, ...) printf(fmt, ## __VA_ARGS__)
#else
#  define DebugPrintf(fmt, ...) ((void)0)
#endif

#define STDIO_BUFFER_SIZE     4096           /**< STDIO buffer size. */
#define MAX_PATH_LENGTH        128           /**< Max path lenght. */       
static char SD_MOUNT_POINT[] = "/mnt/sd0/";  /**< SD mount point. */

namespace SDHCILib {

/**
 * @brief Creates the full path name for the specified relative path name.
 * 
 * @param [out] buf The buffer for full path name.
 * @param [in] bufsize Size of buffer for full path name.
 * @param [in] filepath Relative file path name.
 * @return full path name
 */
static char* fullpathname(char* buf, int bufsize, const char * filepath)
{
  if ((strlen(filepath) + sizeof(SD_MOUNT_POINT) <= (size_t)bufsize) && (bufsize >= 0)) {
    strcpy(buf, SD_MOUNT_POINT);
    strcat(buf, filepath);
    return buf;
  }

  return NULL;
}

File SDClass::open(const char *filepath, uint8_t mode) {
  return File(filepath, mode);
}

boolean SDClass::exists(const char *filepath) {
  struct stat stat;
  char fpbuf[MAX_PATH_LENGTH];
  char *fpname = fullpathname(fpbuf, MAX_PATH_LENGTH, filepath);

  if (fpname) {
    return (::stat(fpname, &stat) == 0);
  } else {
    return false;
  }
}

boolean SDClass::mkdir(const char *filepath) {
  struct stat stat;
  char fpbuf[MAX_PATH_LENGTH];
  char *fpname = fullpathname(fpbuf, MAX_PATH_LENGTH, filepath);
  char *p;
  char tmp;

  if (!fpname)
    return false;

  // create directories recursively
  for (p = fpname + sizeof(SD_MOUNT_POINT); *p; ++p) {
    if (*p == '/' || *(p+1) == 0) {
      tmp = *p;
      *p = 0;
      if (::stat(fpname, &stat) != 0 || !S_ISDIR(stat.st_mode)) {
        if (::mkdir(fpname, 0777) != 0) {
            return false;
        }
        *p = tmp;
      }
    }
  }

  return true;
}

boolean SDClass::rmdir(const char *filepath) {
  char fpbuf[MAX_PATH_LENGTH];
  char *fpname = fullpathname(fpbuf, MAX_PATH_LENGTH, filepath);

  if (!fpname)
    return false;

  return (::rmdir(fpname) == 0);
}

boolean SDClass::remove(const char *filepath) {
  char fpbuf[MAX_PATH_LENGTH];
  char *fpname = fullpathname(fpbuf, MAX_PATH_LENGTH, filepath);

  if (!fpname)
    return false;

  return (::unlink(fpname) == 0);
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

  ret = usbmsc_bindlun(handle, CONFIG_SYSTEM_USBMSC_DEVPATH1, 0, 0, 0, false);
  if (ret < 0)
    {
      DebugPrintf("usbmsc_bindlun failed for LUN 1 using %s: %d\n",
                  CONFIG_SYSTEM_USBMSC_DEVPATH1, -ret);
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

File::File(const char *name, uint8_t mode)
: _name(NULL), _fd(NULL), _size(0), _curpos(0), _dir(NULL) {
  int stat_ret;
  struct stat stat;
  char fpbuf[MAX_PATH_LENGTH];
  char *fpname = fullpathname(fpbuf, MAX_PATH_LENGTH, name);
  String fmode = "";
  String fplus = "";

  if (!fpname)
    return;

  /* Wait for the SD card to be mounted */
  while (::stat(SD_MOUNT_POINT, &stat) < 0);

  stat_ret = ::stat(fpname, &stat);

  if ((stat_ret == 0) && S_ISDIR(stat.st_mode)) {
    _dir = opendir(fpname);
  }
  else {
     /* mode to string (r/w/a|X|b|+)*/

    /* Check Plus case */
    if ((mode & O_RDWR) == O_RDWR) {
      /* Plus */
      fplus += "+";
      if ((mode & O_CREAT) == 0) {
        /* Read */
        fmode += "r";
      } else if (mode & O_APPEND) {
        /* Append */
        fmode += "a";
      } else {
        /* Write */
        fmode += "w";
      }
    } else {
      /* Not Plus */
      if (mode & O_RDOK) {
        /* Read */
        fmode += "r";
      } else if (mode & O_APPEND) {
        /* Append */
        fmode += "a";
      } else {
        /* Write */
        fmode += "w";
      }
    }

    /* Check executable */
    if (mode & O_EXCL) {
      fmode += "X";
    }

    /* Check binary */
    if (mode & O_BINARY) {
      fmode += "b";
    }

    fmode += fplus;

    _fd = ::fopen(fpname, fmode.c_str());
    setvbuf(_fd, NULL, _IOLBF, STDIO_BUFFER_SIZE);
  }

  _name = strdup(name);
  _size = stat.st_size;
  ::fseek(_fd, 0, SEEK_CUR);
  _curpos = ::ftell(_fd);
}

File::File(void):
_name(NULL), _fd(NULL), _size(0), _curpos(0), _dir(NULL) {
}

File::~File() {
}

size_t File::write(const uint8_t *buf, size_t size) {
  size_t wrote;

  if (!_fd) {
    setWriteError();
    return 0;
  }

  wrote = (size_t)::fwrite(buf, sizeof(char), size, _fd);
  if (wrote == size) {
    _curpos += size;
    if (_size < _curpos) {
      _size = _curpos;
    }
    return size;
  }

  setWriteError();
  return 0;
}

size_t File::write(uint8_t data) {
  return write(&data, 1);
}

int File::read() {
  unsigned char byte;
  int ret = read(&byte, 1);
  if (ret == 1) {
    return (int)(unsigned int)byte;
  } else {
    return -1;
  }
}

int File::peek() {
  int pos;
  int byte = -1;

  if (_fd) {
    pos = position();
    byte = read();
    seek(pos);
  }

  return byte;
}

int File::available() {
  if (!_fd) return 0;

  uint32_t n = size() - position();

  return n > 0X7FFF ? 0X7FFF : n;
}

void File::flush() {
  if (_fd)
    fflush(_fd);
}

int File::read(void *buf, uint16_t nbyte) {
  int ret;

  if (_fd) {
    ret = ::fread(buf, sizeof(char), nbyte, _fd);
    if (ret >= 0) {
      _curpos += nbyte;
      return ret;
    }
  }

  return -1;
}

boolean File::seek(uint32_t pos) {
  off_t ofs = -1;

  if (!_fd) return false;

  ofs = ::fseek(_fd, pos, SEEK_SET);
  if (ofs >= 0) {
    _curpos = ::ftell(_fd);
    return true;
  } else {
    return false;
  }
}

uint32_t File::position() {
  if (!_fd) return 0;

  _curpos = ::ftell(_fd);

  return _curpos;
}

uint32_t File::size() {
  if (!_fd) return 0;
  return _size;
}

void File::close() {
  if (_fd) {
    ::fclose(_fd);
    _fd = NULL;
  }

  if (_dir) {
    closedir((DIR*)_dir);
    _dir = NULL;
  }

  if (_name) {
    free(_name);
    _name = NULL;
  }
}

File::operator bool() {
  return (_fd) || (_dir);
}

char * File::name() {
  return _name;
}

boolean File::isDirectory(void) {
  return (_dir != NULL);
}

File File::openNextFile(uint8_t mode) {
  if (_dir) {
    struct dirent * ent = ::readdir((DIR*)_dir);
    if (ent) {
      int l = strlen(_name);
      int sz = l + strlen(ent->d_name) + 2;
      char* name = (char*)malloc(sz);

      if (name) {
        strcpy(name, _name);
        if (name[l - 1] != '/')
            strcat(name, "/");
        strcat(name, ent->d_name);

        File f(name, mode);
        free(name);

        return f;
      }
    }
  }

  return File();
}

void File::rewindDirectory(void) {
  if (_dir)
    ::rewinddir((DIR*)_dir);
}

};
