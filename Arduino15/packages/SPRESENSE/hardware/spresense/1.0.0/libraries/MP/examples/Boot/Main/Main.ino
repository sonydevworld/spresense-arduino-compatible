/*
 *  Main.ino - MP Example to boot multiple SubCores
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

#ifdef SUBCORE
#error "Core selection is wrong!!"
#endif

#include <MP.h>

void setup()
{
  int ret = 0;
  int subid;

  Serial.begin(115200);
  while (!Serial);

  /* Boot SubCore */
  for (subid = 1; subid <= 4; subid++) {
    ret = MP.begin(subid);
    if (ret < 0) {
      MPLog("MP.begin(%d) error = %d\n", subid, ret);
    }
  }
}

void loop()
{
  MPLog("loop\n");
  delay(1000);
}

