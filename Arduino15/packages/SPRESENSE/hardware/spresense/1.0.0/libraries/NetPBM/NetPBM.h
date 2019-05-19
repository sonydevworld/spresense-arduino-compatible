/*
 *  NetPBM.h - NetPBM include file for the Spresense SDK
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

#ifndef Netpbm_h
#define Netpbm_h

#include <File.h>

class NetPBM {
public:
  NetPBM(File& file);
  ~NetPBM();

  size_t size();
  void size(unsigned short *width, unsigned short *height);
  unsigned int getpixel(unsigned short row, unsigned short col);

private:

  int parse();
  int parse_type(char *buf);
  char *getline(char *buf);

  unsigned char *_pixbuf;
  unsigned char *_filebuf;
  unsigned short _width;
  unsigned short _height;
  unsigned char  _bpp;
  unsigned char  _maxvalue;
  bool           _isascii;
};

#endif // Netpbm_h
