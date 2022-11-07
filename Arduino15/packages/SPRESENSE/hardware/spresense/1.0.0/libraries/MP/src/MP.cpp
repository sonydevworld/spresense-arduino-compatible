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
#include <chip/hardware/cxd5602_backupmem.h>
#include <chip/hardware/cxd5602_memorymap.h>
#include <common/arm_internal.h>
#include <armv7-m/nvic.h>
#include <assert.h>
#include <nuttx/arch.h>
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
#ifndef SUBCORE
  memset(_rmng, 0, sizeof(ResourceManagement));
  _rmng->magic = MP_MAGIC;
  sq_init(&_shmlist);
#endif
}

#ifdef SUBCORE
int MPClass::begin()
{
  int ret = 0;

  ret = mpmq_init(&_mq[0], KEY_MQ, 2);
  if (ret == 0) {
    /* boot complete */
    ret = mpmq_send(&_mq[0], 0, 0);
  }

  return ret;
}
#else /* MAINCORE */
int MPClass::begin(int subid)
{
  int ret = 0;

  ret = checkid(subid);
  if ((ret != 0) && (ret != -ENODEV)) {
    return ret;
  }

  /* Wait until RTC is available */
  while (g_rtc_enabled == false);

  if (ret == -ENODEV) {
    ret = load(subid);
    if (ret == 0) {
      uint32_t data;
      /* wait until boot complete */
      ret = mpmq_timedreceive(&_mq[subid], &data, 1000);
    }
  }
  return ret;
}
#endif

#ifdef SUBCORE
int MPClass::end()
{
  /* do nothing */
  return 0;
}
#else /* MAINCORE */
int MPClass::end(int subid)
{
  int ret = 0;

  ret = checkid(subid);
  if (ret) {
    return ret;
  }

  ret = unload(subid);

  return ret;
}
#endif

// send/receive message data
int MPClass::Send(int8_t msgid, uint32_t msgdata, int subid)
{
  int ret;

  ret = checkid(subid);
  if (ret) {
    return ret;
  }

  /* msgid must be 0 or positive value */
  assert(0 <= msgid);

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

  ret = checkid(subid);
  if (ret) {
    return ret;
  }

  ret = mpmq_timedreceive(&_mq[subid], msgdata, _recvTimeout);

  if (ret < 0) {
    //MPDBG("mpmq_timedreceive() failure. %d\n", ret);
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
    MPDBG("sync error: ret=%d data=%ld\n", ret, data);
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
  uint32_t va, pa;
  uint32_t reg;
  int8_t tag;

  va = (uint32_t)virt >> 16;
  if (va & 0x0ff0)
    {
      return (uint32_t)virt;
    }
  tag = va & 0xf;

  uint32_t cpuid = MP_GET_CPUID() - 2;

  reg = CXD56_ADR_CONV_BASE + (cpuid * 0x20) + 4;
  pa = getreg32(reg + (4 * (tag / 2)));

  if (!(tag & 1))
    {
      pa <<= 16;
    }
  pa = (pa & 0x01ff0000u) | ((pa & 0x06000000) << 1);

  return pa | ((uint32_t)virt & 0xffff);
}

#define APPDSP_RAMMODE_STAT0 0x04104420
#define APPDSP_RAMMODE_STAT1 0x04104424

void MPClass::GetMemoryInfo(int &usedMem, int &freeMem, int &largestFreeMem)
{
  int i;
  uint32_t tile = (getreg32(APPDSP_RAMMODE_STAT1) << 12) | getreg32(APPDSP_RAMMODE_STAT0);
  int prev_free = 0;
  int tmp_largest = 0;

  usedMem = 0;
  freeMem = 0;
  largestFreeMem = 0;

  for (i = 0; i < 12; i++) {
    if (tile & (1 << (i * 2))) {
      usedMem++;
      prev_free = 0;
    } else {
      freeMem++;
      if (prev_free) {
        tmp_largest++;
      } else {
        tmp_largest = 1;
      }
      prev_free = 1;
    }

    if (largestFreeMem < tmp_largest) {
      largestFreeMem = tmp_largest;
    }
  }
  usedMem *= (128 * 1024);
  freeMem *= (128 * 1024);
  largestFreeMem *= (128 * 1024);
}

void MPClass::EnableConsole()
{
  /* Enable console interrupts */
  int irq = CXD56_IRQ_UART1 - CXD56_IRQ_EXTINT;
  uint32_t bit = 1 << (irq & 0x1f);
  putreg32(bit, NVIC_IRQ_ENABLE(irq));
}

void MPClass::DisableConsole()
{
  /* Disable console interrupts */
  int irq = CXD56_IRQ_UART1 - CXD56_IRQ_EXTINT;
  uint32_t bit = 1 << (irq & 0x1f);
  putreg32(bit, NVIC_IRQ_CLEAR(irq));
}

#ifndef SUBCORE
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
#endif /* !SUBCORE */

/****************************************************************************
 * Private Functions
 ****************************************************************************/

#ifdef SUBCORE
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

  if (_mq[subid].cpuid == 0) {
    return -ENODEV;
  }

  return 0;
}
#else /* MAINCORE */
int MPClass::checkid(int subid)
{
  if ((subid < 1) || (MP_MAX_SUBID <= subid)) {
    return -EINVAL;
  }

  if (_mq[subid].cpuid == 0) {
    return -ENODEV;
  }

  return 0;
}
#endif

#ifndef SUBCORE
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
  memset(&_mq[subid], 0, sizeof(mpmq_t));

  /* Unregister cpuid assignment */

  _rmng->cpu_assign &= ~CLR_CPU(subid);

  return ret;
}
#endif

