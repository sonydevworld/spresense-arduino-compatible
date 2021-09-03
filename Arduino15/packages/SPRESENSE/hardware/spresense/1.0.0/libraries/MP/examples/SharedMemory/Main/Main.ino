/*
 *  Main.ino - MP Example for MP Shared Memory
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

#define MEMSIZE (128 * 1024) // 128KB

int subcore = 1;

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  /* Boot SubCore */
  MP.begin(subcore);

  /* Allocate Shared Memory */
  uint8_t *addr = (uint8_t *)MP.AllocSharedMemory(MEMSIZE);
  if (!addr) {
    printf("Error: out of memory\n");
    return;
  }

  printf("SharedMemory Address=@%08lx\n", (uint32_t)addr);

  /* memory fill */
  memset(addr, 0x55, MEMSIZE);

  /* Send shared memory address to SubCore */
  int8_t msgid = 10;
  MP.Send(msgid, addr, subcore);

  /* Receive from SubCore */
  void *raddr;
  MP.Recv(&msgid, &raddr, subcore);

  /* shared memory check */
  int i;
  for (i = 0; i < MEMSIZE; i++) {
    if (addr[i] != 0xaa) {
      printf("Error: @%08lx\n", (uint32_t)&addr[i]);
      while (1);
    }
  }
  printf("SharedMemory Check: OK!\n");

  /* Free Shared Memory */
  MP.FreeSharedMemory(addr);
}

void loop()
{
}

