/*
 *  MP.cpp - Spresense Arduino Multi-Processer Communication library
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sdk/config.h>
#include <stdio.h>
#include "MP.h"

/****************************************************************************
 * Public Data
 ****************************************************************************/

MPClass MP;

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int MPClass::begin(int subid)
{
  int ret;

  if (checkid(subid)) {
    return -EINVAL;
  }
#ifdef CONFIG_CXD56_SUBCORE
  ret = mpmq_init(&_mq[subid], KEY_MQ, 2);
  if (ret == 0) {
    /* boot complete */
    ret = mpmq_send(&_mq[subid], 0, 0);
  }
#else
  ret = load(subid);
  if (ret == 0) {
    uint32_t data;
    /* wait until boot complete */
    ret = mpmq_timedreceive(&_mq[subid], &data, 1000);
  }
#endif
  return ret;
}

int MPClass::end(int subid)
{
  int ret = 0;

  if (checkid(subid)) {
    return -EINVAL;
  }
#ifndef CONFIG_CXD56_SUBCORE
  ret = unload(subid);
#endif
  return ret;
}

// send/receive message data
int MPClass::Send(int8_t msgid, uint32_t msgdata, int subid)
{
  int ret;

  if (checkid(subid)) {
    return -EINVAL;
  }

  ret = mpmq_send(&_mq[subid], msgid, msgdata);

  if (ret < 0){
    MPDBG("mpmq_send() failure. %d\n", ret);
    return ret;
  }

  return ret;
}

int MPClass::Recv(int8_t *msgid, uint32_t *msgdata, int subid)
{
  int ret;

  if (checkid(subid)) {
    return -EINVAL;
  }

  ret = mpmq_timedreceive(&_mq[subid], msgdata, _recvTimeout);

  if (ret < 0) {
    MPDBG("mpmq_timedreceive() failure. %d\n", ret);
    return ret;
  }

  *msgid = (int8_t)ret;

  return ret;
}

// send/receive message address
int MPClass::Send(int8_t msgid, void *msgaddr, int subid)
{
  return Send(msgid, Virt2Phys(msgaddr), subid);
}

int MPClass::Recv(int8_t *msgid, void *msgaddr, int subid)
{
  return Recv(msgid, (uint32_t*)msgaddr, subid);
}

// send/receive message object
int MPClass::SendWaitComplete(int subid)
{
  int ret;
  int8_t msgid;
  uint32_t data;

  ret = Recv(&msgid, &data, subid);
  if ((ret != 0) || (data != 0)) {
    MPDBG("sync error: ret=%d data=%d\n", ret, data);
    return ret;
  }

  return ret;
}

// receive timeout
void MPClass::RecvTimeout(uint32_t timeout)
{
  _recvTimeout = timeout;
}

uint32_t MPClass::GetRecvTimeout(void)
{
  return _recvTimeout;
}

// convert virtual to physical address
uint32_t MPClass::Virt2Phys(void *virt)
{
  uint32_t phys;

  if ((uint32_t)virt < 0x00100000) {
    phys = mpshm_virt2phys(NULL, virt);
  } else {
    phys = (uint32_t)virt;
  }

  return phys;
}

/****************************************************************************
 * Private Functions
 ****************************************************************************/

int MPClass::checkid(int subid)
{
#ifdef CONFIG_CXD56_SUBCORE
  if (subid != 0) {
    return -EINVAL;
  }
#else
  if ((subid < 1) || (MP_MAX_SUBID <= subid)) {
    return -EINVAL;
  }
#endif
  return 0;
}

#ifndef CONFIG_CXD56_SUBCORE
int MPClass::load(int subid)
{
  int ret;
  char filename[5];

  snprintf(filename, sizeof(filename), "sub%d", subid);

  /* Initialize MP task */

  ret = mptask_init_secure(&_mptask[subid], filename);

  if (ret != 0) {
    MPDBG("mptask_init() failure. %d\n", ret);
    return ret;
  }

  ret = mptask_assign(&_mptask[subid]);

  if (ret != 0) {
    MPDBG("mptask_assign() failure. %d\n", ret);
    return ret;
  }

  /* Initialize MP message queue with assigned CPU ID, and bind it to MP task */

  ret = mpmq_init(&_mq[subid], KEY_MQ, mptask_getcpuid(&_mptask[subid]));

  if (ret < 0) {
    MPDBG("mpmq_init() failure. %d\n", ret);
    return ret;
  }

  /* Run SubCore */

  ret = mptask_exec(&_mptask[subid]);
  if (ret < 0) {
    MPDBG("mptask_exec() failure. %d\n", ret);
    return ret;
  }

  return 0;
}

int MPClass::unload(int subid)
{
  int ret = 0, wret;

  ret = mptask_destroy(&_mptask[subid], false, &wret);
  mpmq_destroy(&_mq[subid]);

  return ret;
}
#endif

