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
#include <chip/cxd5602_backupmem.h>
#include "MP.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define BACKUP_MEM 0x04400070
#define MP_MAGIC   0x4d52504d

#define GET_CPU(subid)      ((_rmng->cpu_assign >> ((subid) * 3)) & 7)
#define SET_CPU(subid, cpu) (((cpu) & 7) << ((subid) * 3))
#define CLR_CPU(subid)      (7 << ((subid) * 3))

/****************************************************************************
 * Public Data
 ****************************************************************************/

MPClass MP;

/****************************************************************************
 * Public Functions
 ****************************************************************************/

MPClass::MPClass() : _recvTimeout(MP_RECV_BLOCKING)
{
  memset(_mq, 0, sizeof(_mq));
  _rmng = (struct ResourceManagement*)BACKUP_MEM;
#ifndef CONFIG_CXD56_SUBCORE
  memset(_rmng, 0, sizeof(ResourceManagement));
  _rmng->magic = MP_MAGIC;
  sq_init(&_shmlist);
#endif
}

int MPClass::begin(int subid)
{
  int ret = 0;

  if (checkid(subid)) {
    return -EINVAL;
  }
#ifdef CONFIG_CXD56_SUBCORE
  if (subid == 0) {
    ret = mpmq_init(&_mq[subid], KEY_MQ, 2);
    if (ret == 0) {
      /* boot complete */
      ret = mpmq_send(&_mq[subid], 0, 0);
    }
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

#ifndef CONFIG_CXD56_SUBCORE
void *MPClass::AllocSharedMemory(size_t size)
{
  void    *virt;
  int ret;

  if (size == 0) {
    return NULL;
  }

  shm_entry *node = (shm_entry *)malloc(sizeof(shm_entry));
  if (!node) {
    return NULL;
  }

  ret = mpshm_init(&node->shm, KEY_SHM, size);
  if (ret < 0) {
    free(node);
    return NULL;
  }

  virt = mpshm_attach(&node->shm, 0);
  if (!virt) {
    mpshm_destroy(&node->shm);
    free(node);
    return NULL;
  }

  node->addr = mpshm_virt2phys(NULL, virt);

  sq_addfirst(&node->entry, &_shmlist);

  return (void *)node->addr;
};

void MPClass::FreeSharedMemory(void *addr)
{
  sq_entry_t *entry;
  for (entry = sq_peek(&_shmlist); entry; entry = sq_next(entry)) {
    if (((shm_entry *)entry)->addr == (uint32_t)addr) {
      sq_rem(entry, &_shmlist);
      break;
    }
  }
  if (entry) {
    shm_entry *node = (shm_entry *)entry;
    mpshm_detach(&node->shm);
    mpshm_destroy(&node->shm);
    free(node);
  }
}
#endif /* !CONFIG_CXD56_SUBCORE */

/****************************************************************************
 * Private Functions
 ****************************************************************************/

#ifdef CONFIG_CXD56_SUBCORE
int MPClass::checkid(int subid)
{
  int ret;

  if ((subid < 0) || (MP_MAX_SUBID <= subid)) {
    return -EINVAL;
  }

  if (subid == SUBCORE) {
    return -EINVAL;
  }

  if (subid == 0) {
    return 0;
  }

  /* Communicate between SubCore(s) */
  if (_mq[subid].cpuid == 0) {
    /* Get cpuid assignment */
    cpuid_t cpu = GET_CPU(subid);
    if ((cpu == 0) || (cpu == MP_GET_CPUID())) {
      return -ENXIO;
    }
    /* create a message queue */
    ret = mpmq_init(&_mq[subid], KEY_MQ, cpu);
    if (ret < 0) {
      return ret;
    }
  }

  return 0;
}
#else
int MPClass::checkid(int subid)
{
  if ((subid < 1) || (MP_MAX_SUBID <= subid)) {
    return -EINVAL;
  }
  return 0;
}
#endif

#ifndef CONFIG_CXD56_SUBCORE
int MPClass::load(int subid)
{
  int ret;
  char filename[5];
  cpuid_t cpu;

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

  cpu = mptask_getcpuid(&_mptask[subid]);
  ret = mpmq_init(&_mq[subid], KEY_MQ, cpu);

  if (ret < 0) {
    MPDBG("mpmq_init() failure. %d\n", ret);
    return ret;
  }

  /* Register cpuid assignment */

  _rmng->cpu_assign |= SET_CPU(subid, cpu);

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

  /* Unregister cpuid assignment */

  _rmng->cpu_assign &= ~CLR_CPU(subid);

  return ret;
}
#endif
