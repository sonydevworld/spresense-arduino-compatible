/*
 *  multi_print.c - Spresense MultiCore printlog functions
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

#include <sdk/config.h>
#include <nuttx/streams.h>
#include <common/up_internal.h>
#include <stdio.h>
#include <string.h>
#include <cxd56_sph.h>
#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <common/up_arch.h>
#include <armv7-m/nvic.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "multi_print.h"

static int printlock_fd;

void init_multi_print(void)
{
#ifdef SUBCORE
  /* Disable console interrupts for SubCore by default */
  int irq = CXD56_IRQ_UART1 - CXD56_IRQ_EXTINT;
  uint32_t bit = 1 << (irq & 0x1f);
  putreg32(bit, NVIC_IRQ_CLEAR(irq));
#endif
  /* Initialize hardware semaphore for exclusive control of print log */
  printlock_fd = open("/dev/hsem3", 0);
}

void printlock(void)
{
  int ret;
  if (!up_interrupt_context()) {
    do {
      ret = ioctl(printlock_fd, HSTRYLOCK, 0);
    } while (ret != 0);
  }
}

void printunlock(void)
{
  if (!up_interrupt_context()) {
    ioctl(printlock_fd, HSUNLOCK, 0);
  }
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

    up_lowputc(ch);
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
