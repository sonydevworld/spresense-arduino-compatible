/*
 *  multi_print.c - Spresense MultiCore printlog functions
 *  Copyright 2019,2021 Sony Semiconductor Solutions Corporation
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

#include <sdk/config.h>
#include <nuttx/streams.h>
#include <common/arm_internal.h>
#include <stdio.h>
#include <string.h>
#include <cxd56_sph.h>
#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <common/arm_internal.h>
#include <armv7-m/nvic.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <hardware/cxd56_sph.h>
#include "multi_print.h"

#define PRINT_HSEMID 3
#define CPU_ID (CXD56_CPU_BASE + 0x40)

#define sph_state_unlocked(sts) (STS_STATE(sts) == STATE_IDLE)
#define sph_state_locked(sts)   (STS_STATE(sts) == STATE_LOCKED)
#define sph_state_busy(sts)     (STS_STATE(sts) == STATE_LOCKEDANDRESERVED)

static uint32_t g_cpuid;

void init_multi_print(void)
{
#ifdef SUBCORE
  /* Disable console interrupts for SubCore by default */
  int irq = CXD56_IRQ_UART1 - CXD56_IRQ_EXTINT;
  uint32_t bit = 1 << (irq & 0x1f);
  putreg32(bit, NVIC_IRQ_CLEAR(irq));
#endif
  /* Save cpuid to use the hardware semaphore */
  g_cpuid = getreg32(CPU_ID);
}

irqstate_t printlock(void)
{
  irqstate_t flags;
  uint32_t   sts;

  flags = enter_critical_section();
  for (int i = 0; i < INT_MAX; i++) {
    sts = getreg32(CXD56_SPH_STS(PRINT_HSEMID));
    if (sph_state_unlocked(sts)) {
      putreg32(REQ_LOCK, CXD56_SPH_REQ(PRINT_HSEMID));

      sts = getreg32(CXD56_SPH_STS(PRINT_HSEMID));
      if (sph_state_locked(sts) && (LOCK_OWNER(sts) == g_cpuid)) {
        return flags;
      }
    }
  }
  /* Any system fatal error to assert */
  assert(0);
  return flags;
}

void printunlock(irqstate_t flags)
{
  putreg32(REQ_UNLOCK, CXD56_SPH_REQ(PRINT_HSEMID));
  leave_critical_section(flags);
}

/*
 * uart write with synchronous
 */

ssize_t uart_syncwrite(const char *buffer, size_t buflen)
{
  ssize_t ret = buflen;

  /* Force each character through the low level interface */

  for (; buflen; buflen--) {
    int ch = *buffer++;

    /* Output the character, using the low-level direct UART interfaces */

    arm_lowputc(ch);
  }

  return ret;
}

/*
 * sync_printf for exclusive control
 */

int sync_printf(const char *fmt, ...)
{
  struct lib_memoutstream_s memoutstream;
  va_list ap;
  char buf[128];
  int n;

  lib_memoutstream(&memoutstream, buf, sizeof(buf));

  va_start(ap, fmt);
  n = lib_vsprintf((FAR struct lib_outstream_s *)&memoutstream.public, fmt, ap);
  va_end(ap);

  uart_syncwrite(buf, n);
  return n;
}

#ifdef SUBCORE

/* Always replace printf to sync_printf for SubCore */
__attribute((weak, alias("sync_printf"))) int printf(const char *fmt, ...);

/* Always use the specific puts for SubCore */
int puts(const char *s)
{
  char buf[128];
  int n;

  n = strlen(s);

  if (n == 0)
    return EOF;

  if (n > 127)
    n = 127;

  memcpy(buf, s, n);
  buf[n] = '\n';

  uart_syncwrite(buf, n + 1);
  return n + 1;
}

#endif /* SUBCORE */
