/*
 *  Main.ino - MP Example to communicate message data
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

int subcore = 1; /* Communication with SubCore1 */

void setup()
{
  int ret = 0;

  Serial.begin(115200);
  while (!Serial);

  /* Launch SubCore1 */
  ret = MP.begin(subcore);
  if (ret < 0) {
    printf("MP.begin error = %d\n", ret);
  }

  randomSeed(100);
}

void loop()
{
  int      ret;
  uint32_t snddata;
  uint32_t rcvdata;
  int8_t   sndid = 100; /* user-defined msgid */
  int8_t   rcvid;

  snddata = random(32767);

  /* Echo back from SubCore */

  printf("Send: id=%d data=0x%08lx\n", sndid, snddata);

  ret = MP.Send(sndid, snddata, subcore);
  if (ret < 0) {
    printf("MP.Send error = %d\n", ret);
  }

  /* Timeout 1000 msec */
  MP.RecvTimeout(1000);

  ret = MP.Recv(&rcvid, &rcvdata, subcore);
  if (ret < 0) {
    printf("MP.Recv error = %d\n", ret);
  }

  printf("Recv: id=%d data=0x%08lx : %s\n", rcvid, rcvdata,
         (snddata == rcvdata) ? "Success" : "Fail");

  delay(1000);
}

