/*
 *  Sub3.ino - MP Example for MP Mutex
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

#if (SUBCORE != 3)
#error "Core selection is wrong!!"
#endif

#include <MP.h>
#include <MPMutex.h>

/* Create a MPMutex object */
MPMutex mutex(MP_MUTEX_ID0);

void setup()
{
  MP.begin();
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
    ledOn(LED3);
    delay(500);
    ledOff(LED3);
    delay(500);
  }

  /* Unlock the mutex */
  mutex.Unlock();
}

