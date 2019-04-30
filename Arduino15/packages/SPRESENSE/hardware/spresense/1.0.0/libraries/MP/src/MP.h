/*
 *  MP.h - Spresense Arduino Multi-Processer Communication library
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

#ifndef _MP_H_
#define _MP_H_

/**
 * @file MP.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino Multi-Processer Communication library
 *
 * @details The MP library can manage the Multi-processor communication.
 */

/**
 * @defgroup mp MP Library API
 * @brief API for using MP API
 * @{
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <Arduino.h>
#include <sdk/config.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <asmp/asmp.h>
#include <asmp/mptask.h>
#include <asmp/mpshm.h>
#include <asmp/mpmq.h>
#include <asmp/mpmutex.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef BRD_DEBUG
#define MPDBG(format, ...) printf("DEBUG: " format, ##__VA_ARGS__)
#else
#define MPDBG(format, ...)
#endif
#define MPERR(format, ...) printf("ERROR: " format, ##__VA_ARGS__)

#define KEY_MQ    2

#define MP_RECV_BLOCKING    (0)
#define MP_RECV_POLLING     (MPMQ_NONBLOCK)

#define MP_GET_CPUID()      (*(volatile int *)0x4e002040)

#define MP_MAX_SUBID 6

/****************************************************************************
 * class declaration
 ****************************************************************************/

/**
 * @class MPClass
 * @brief This is the interface for MP (Multi-Processor).
 *
 */
class MPClass
{
public:
  MPClass();

  int begin(int subid = 0);
  int end(int subid = 0);

  // send/receive message data
  int Send(int8_t msgid, uint32_t msgdata, int subid = 0);
  int Recv(int8_t *msgid, uint32_t *msgdata, int subid = 0);

  // send/receive message address
  int Send(int8_t msgid, void *msgaddr, int subid = 0);
  int Recv(int8_t *msgid, void *msgaddr, int subid = 0);

  // send/receive message object
  template <typename T> int SendObject(T &t, int subid = 0);
  template <typename T> int RecvObject(T &t, int subid = 0);
  int SendWaitComplete(int subid = 0);

  // receive timeout
  void     RecvTimeout(uint32_t timeout);
  uint32_t GetRecvTimeout();

  // convert virtual to physical address
  uint32_t Virt2Phys(void *virt);

private:
  uint32_t _recvTimeout;
  mpmq_t   _mq[MP_MAX_SUBID];
  struct ResourceManagement {
    uint32_t magic;
    uint32_t cpu_assign;
    uint32_t reserved[2];
    uint32_t resource[4];
  } *_rmng;

  int checkid(int subid);
#ifndef CONFIG_CXD56_SUBCORE
  mptask_t _mptask[MP_MAX_SUBID];
  int load(int subid);
  int unload(int subid);
#endif
};

/****************************************************************************
 * template functions
 ****************************************************************************/

// send/receive message object
template <typename T> int MPClass::SendObject(T &t, int subid)
{
  int ret;

  if (checkid(subid)) {
    return -EINVAL;
  }

  size_t msgsz = sizeof(T);
  if (msgsz > 127) {
    return -EINVAL;
  }

  ret = Send((int8_t)msgsz, Virt2Phys(&t), subid);
  if (ret < 0) {
    MPDBG("Send(&object) failure. %d\n", ret);
    return ret;
  }

  return ret;
}

template <typename T> int MPClass::RecvObject(T &t, int subid)
{
  int ret;
  void *vp;
  int8_t rsz;

  if (checkid(subid)) {
    return -EINVAL;
  }

  size_t msgsz = sizeof(T);
  if (msgsz > 127) {
    return -EINVAL;
  }

  ret = Recv(&rsz, (uint32_t*)&vp, subid);

  if ((ret <= 0) || (msgsz != ret)) {
    MPDBG("Recv(&object) failure. %d\n", ret);
    mpmq_send(&_mq[subid], -1, 0); // error
    return ret;
  }

  memcpy(&t, vp, msgsz);

  ret = mpmq_send(&_mq[subid], 0, 0); // success
  if (ret < 0) {
    MPDBG("mpmq_send() failure. %d\n", ret);
    return ret;
  }

  return ret;
}

/****************************************************************************
 * extern declaration
 ****************************************************************************/

extern MPClass MP;

/** @} mp */

#endif /* _MP_H_ */
