/*
 *  I2cScanner.ino - Wire(I2C) 7-bit slave address scanning tool
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

#include <Wire.h>

void setup()
{
  Wire.begin();
  Serial.begin(115200);
}


void loop()
{
  uint8_t error;
  int nDevices;
  int addr;
  int upper, lower;

  printf("I2C Scanning...\n\n");

  nDevices = 0;
  printf("     -0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -A -B -C -D -E -F\n");
  for (upper = 0; upper < 8; upper++) {
    printf("%d- :", upper);
    for (lower = 0; lower < 16; lower++) {
      addr = upper * 16 + lower;
      Wire.beginTransmission(addr);
      error = Wire.endTransmission();
      if (error) {
        printf(" --");
      } else {
        printf(" %02x", addr);
        nDevices++;
      }
    }
    printf("\n");
  }

  printf("\nthe number of found I2C devices is %d.\n\n", nDevices);

  delay(5000);
}
