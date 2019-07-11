/*
 *  Sub1.ino - MP Example for MP Shared Memory
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

#if (SUBCORE != 1)
#error "Core selection is wrong!!"
#endif

#include <MP.h>

#define MEMSIZE (128 * 1024) // 128KB

void setup()
{
  int8_t msgid;
  void *addr;

  MP.begin();

  /* Receive from MainCore */
  MP.Recv(&msgid, &addr);

  /* memory fill */
  memset(addr, 0xaa, MEMSIZE);

  /* Send address to MainCore */
  MP.Send(msgid, addr);
}

void loop()
{
}

