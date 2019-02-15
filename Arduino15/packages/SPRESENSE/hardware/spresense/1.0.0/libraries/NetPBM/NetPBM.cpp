/*
 *  NetPBM.cpp - NetPBM implementation file for the Spresense SDK
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <Arduino.h>
#include <NetPBM.h>

NetPBM::NetPBM(File& file) :
  _pixbuf(0),
  _width(0),
  _height(0),
  _bpp(0),
  _maxvalue(0),
  _isascii(false)
{

  uint32_t size = file.size() + 1;

  _filebuf = (unsigned char *)malloc(size);
  if (_filebuf == NULL)
    {
      return;
    }

  file.read(_filebuf, size);
  _filebuf[size - 1] = '\0';

  int ret = this->parse();
  if (ret < 0)
    {
      return;
    }

  if (_isascii)
    {
      // Sorry, now ASCII format is not supported.
    }
  else
    {
      _pixbuf = _filebuf + ret;
    }
}

NetPBM::~NetPBM()
{
  if (_filebuf)
    {
      free(_filebuf);
    }
}

char *
NetPBM::getline(char *buf)
{
  char *p = buf;

  do
    {
      p = strchr(p, '\n');
      if (!p)
        {
          break;
        }
      *p++ = '\0';
    }
  while (*p == '#');

  return p;
}

int
NetPBM::parse()
{
  int ret;
  char *cur = (char *)_filebuf;
  char *next;

  // Read 3 lines for determine map type, width x line and maxvalie

  next = this->getline(cur);
  ret = this->parse_type(cur);
  if (ret < 0)
    {
      return -1;
    }
  cur = next;

  next = this->getline(cur);
  ret = sscanf(cur, "%hd %hd", &_width, &_height);
  if (ret < 2)
    {
      return -1;
    }
  cur = next;

  if (_bpp != 1)
    {
      next = this->getline(cur);
      _maxvalue = atoi(cur);
      cur = next;
    }

  // Return start offset of pixel data

  return (unsigned char *)cur - _filebuf;
}

int
NetPBM::parse_type(char *buf)
{
  if (buf[0] != 'P')
    {
      return -1;
    }
  switch (buf[1])
    {
      case '1':
        _isascii = true;
        // FALLTHRU
      case '4':
        _bpp = 1;
        break;

      case '2':
        _isascii = true;
        // FALLTHRU
      case '5':
        _bpp = 8;
        break;

      case '3':
        _isascii = true;
        // FALLTHRU
      case '6':
        _bpp = 24;
        break;
      default:
        return -2;
    }

  return 0;
}

size_t
NetPBM::size()
{
  return _width * _height * _bpp / 8;
}

void
NetPBM::size(unsigned short *width, unsigned short *height)
{
  *width = _width;
  *height = _height;
}

unsigned int
NetPBM::getpixel(unsigned short row, unsigned short col)
{
  size_t offset = (row * _width) + col;

  if (_pixbuf)
    {
      // XXX: 8 bpp gray map only
      return (unsigned int)_pixbuf[offset];
    }
  else
    {
      return 0;
    }
}
