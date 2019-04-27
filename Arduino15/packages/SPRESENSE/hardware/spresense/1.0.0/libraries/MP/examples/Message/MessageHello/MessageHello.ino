/*
 *  MessageHello.ino - MP Example to communicate message strings
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

#include <MP.h>

#define MSGLEN      64
#define MY_MSGID    10
struct MyPacket {
  volatile int status; /* 0:ready, 1:busy */
  char message[MSGLEN];
};

#ifdef SUBCORE

#if   (SUBCORE == 1)
#define SUBID "Sub1"
#elif (SUBCORE == 2)
#define SUBID "Sub2"
#elif (SUBCORE == 3)
#define SUBID "Sub3"
#elif (SUBCORE == 4)
#define SUBID "Sub4"
#elif (SUBCORE == 5)
#define SUBID "Sub5"
#endif

MyPacket packet;

void setup()
{
  memset(&packet, 0, sizeof(packet));
  MP.begin();
}

void loop()
{
  int        ret;
  static int count = 0;

  if (packet.status == 0) {

    /* status -> busy */
    packet.status = 1;

    /* Create a message */
    snprintf(packet.message, MSGLEN, "[%s] Hello %d", SUBID, count++);

    /* Send to MainCore */
    ret = MP.Send(MY_MSGID, &packet);
    if (ret < 0) {
      printf("MP.Send error = %d\n", ret);
    }
  }

  delay(500);
}

#else  /* MAINCORE */

void setup()
{
  int ret = 0;
  int subid;

  Serial.begin(115200);
  while (!Serial);

  /* Boot SubCore */
  for (subid = 1; subid <= 5; subid++) {
    ret = MP.begin(subid);
    if (ret < 0) {
      printf("MP.begin(%d) error = %d\n", subid, ret);
    }
  }

  /* Polling */
  MP.RecvTimeout(MP_RECV_POLLING);
}

void loop()
{
  int      ret;
  int      subid;
  int8_t   msgid;
  MyPacket *packet;

  /* Receive message from SubCore */
  for (subid = 1; subid <= 5; subid++) {
    ret = MP.Recv(&msgid, &packet, subid);
    if (ret > 0) {
      printf("%s\n", packet->message);
      /* status -> ready */
      packet->status = 0;
    }
  }
}

#endif

