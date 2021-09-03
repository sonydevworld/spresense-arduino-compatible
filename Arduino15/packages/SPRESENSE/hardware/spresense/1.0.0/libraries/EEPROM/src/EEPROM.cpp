/*
  EEPROM.cpp - EEPROM library
  Copyright 2018 Sony Semiconductor Solutions Corporation

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "EEPROM.h"

// Read a byte from the address specified by index on a eeprom emulation file
uint8_t EERef::operator*() const
{
  int ret;
  FILE *fp = NULL;
  uint8_t value = 0;

  if ((fp = fopen(EEPROM_EMU, "rb")) == NULL) {
    printf("ERROR: eeprom open failure\n");
    goto errout;
  }

  ret = fseek(fp, index, SEEK_SET);
  if (ret) {
    printf("ERROR: eeprom seek failure\n");
    goto errout_with_close;
  }

  ret = fread(&value, 1, 1, fp);
  if (ret != 1) {
    printf("ERROR: eeprom read failure (%d)\n", ret);
  }
errout_with_close:
  fclose(fp);
errout:
  return value;
}

// Write a byte to the address specified by index on a eeprom emulation file
EERef& EERef::operator=( uint8_t in )
{
  int ret;
  FILE *fp = NULL;

  if ((fp = fopen(EEPROM_EMU, "rb+")) == NULL) {
    printf("ERROR: eeprom open failure\n");
    goto errout;
  }

  ret = fseek(fp, index, SEEK_SET);
  if (ret) {
    printf("ERROR: eeprom seek failure\n");
    goto errout_with_close;
  }

  ret = fwrite(&in, 1, 1, fp);
  if (ret != 1) {
    printf("ERROR: eeprom write failure (%d)\n", ret);
  }

errout_with_close:
  fclose(fp);
errout:
  return *this;
}

EEPROMClass EEPROM;
