/*
 *  File.cpp - Spresense Arduino file library
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
 * @file File.cpp
 * @author Sony Semiconductor Solutions Corporation
 * @brief SPRESENSE Arduino file library
 * 
 * @details The File library allows for reading from and writing a file
 */

#include <sdk/config.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

#include <File.h>

#define MAXFILELEN 128

//#define DEBUG
#ifdef DEBUG
#  define DebugPrintf(fmt, ...) ::printf(fmt, ## __VA_ARGS__)
#else
#  define DebugPrintf(fmt, ...) ((void)0)
#endif

File::File(const char *name, uint8_t mode)
: _name(NULL), _fd(-1), _size(0), _curpos(0), _dir(NULL) {
  int stat_ret;
  struct stat stat;
  char fpbuf[MAXFILELEN];
  int retry = 0;

  if (!name)
    return;

  /*
   * For backward compatibility
   */
  if (0 != strncmp(name, "/mnt/", 5)) {
    /* Treat File without '/mnt/' as a file on SD card */
    strncpy(fpbuf, "/mnt/sd0/", sizeof(fpbuf));
    strncat(fpbuf, name, sizeof(fpbuf) - 10);
    name = fpbuf;
  }

  if (0 == strncmp(name, "/mnt/sd0/", 9)) {
    /* Wait for the SD card to be mounted */
    while (::stat("/mnt/sd0/", &stat) < 0) {
      retry++;
      if (retry >= 20) {
        retry = 0;
        ::printf("Insert SD card!\n");
      }
      usleep(100 * 1000); // 100 msec
    }
  }

  /* initialize variable in case stat() returns an error */
  stat.st_size = 0;

  stat_ret = ::stat(name, &stat);

  if ((stat_ret == 0) && S_ISDIR(stat.st_mode)) {
    _dir = opendir(name);
  }
  else {
    _fd = ::open(name, mode);
  }

  _name = strdup(name);
  if (_fd >= 0) {
    _size = stat.st_size;
    if (mode == FILE_WRITE) {
      _curpos = ::lseek(_fd, 0, SEEK_END);
    } else {
      _curpos = ::lseek(_fd, 0, SEEK_CUR);
    }
  }
}

File::File(void):
_name(NULL), _fd(-1), _size(0), _curpos(0), _dir(NULL) {
}

File::~File() {
}

size_t File::write(const uint8_t *buf, size_t size) {
  size_t wrote;

  if (_fd < 0) {
    setWriteError();
    return 0;
  }

  wrote = (size_t)::write(_fd, buf, size);
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
    return (int)byte;
  } else {
    return -1;
  }
}

int File::peek() {
  int pos;
  int byte = -1;

  if (_fd >= 0) {
    pos = position();
    byte = read();
    seek(pos);
  }

  return byte;
}

int File::available() {
  if (_fd < 0) return 0;

  return size() - position();
}

void File::flush() {
  if (_fd >= 0)
    fsync(_fd);
}

int File::read(void *buf, size_t nbyte) {
  int ret;

  if (_fd >= 0) {
    ret = ::read(_fd, buf, nbyte);
    if (ret >= 0) {
      _curpos += nbyte;
      return ret;
    }
  }

  return -1;
}

boolean File::seek(uint32_t pos) {
  off_t ofs = -1;

  if (_fd < 0) return false;

  ofs = ::lseek(_fd, pos, SEEK_SET);
  if (ofs >= 0) {
    _curpos = ofs;
    return true;
  } else {
    return false;
  }
}

uint32_t File::position() {
  if (!_fd) return 0;

  _curpos = ::lseek(_fd, 0, SEEK_CUR);

  return _curpos;
}

uint32_t File::size() {
  if (_fd < 0) return 0;
  return _size;
}

void File::close() {
  if (_fd >= 0) {
    ::close(_fd);
    _fd = -1;
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
  return (_fd >= 0) || (_dir);
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
      if ((sz <= 0) || (MAXFILELEN <= sz)) {
        goto error;
      }
      char* name = (char*)malloc(sz);

      if (name) {
        strncpy(name, _name, l + 1);
        if (name[l - 1] != '/')
            strncat(name, "/", 2);
        strncat(name, ent->d_name, sz - strlen(name) - 1);

        File f(name, mode);
        free(name);

        return f;
      }
    }
  }
error:
  return File();
}

void File::rewindDirectory(void) {
  if (_dir)
    ::rewinddir((DIR*)_dir);
}

