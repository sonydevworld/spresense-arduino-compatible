/*
 *  Main.ino - MP Example for MP Mutex
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
#include <MPMutex.h>

/* Create a MPMutex object */
MPMutex mutex(MP_MUTEX_ID0);

void setup()
{
  int ret = 0;
  int subid;

  Serial.begin(115200);
  while (!Serial);

  /* Boot SubCore */
  for (subid = 1; subid <= 3; subid++) {
    ret = MP.begin(subid);
    if (ret < 0) {
      printf("MP.begin(%d) error = %d\n", subid, ret);
    }
  }
}

void loop()
{
  int cnt = 3;
  int ret;

  /* Busy wait until lock the mutex */
  do {
    ret = mutex.Trylock();
  } while (ret != 0);

  /* If the mutex is acquired, blink LED */
  MPLog("Lock\n");
  while (cnt--) {
    ledOn(LED0);
    delay(500);
    ledOff(LED0);
    delay(500);
  }

  /* Unlock the mutex */
  mutex.Unlock();
}

