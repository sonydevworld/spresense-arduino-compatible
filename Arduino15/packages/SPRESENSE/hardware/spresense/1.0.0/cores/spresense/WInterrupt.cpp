/*
  WInterrupt.cpp - Interrupts API implementation file for the Sparduino SDK
  Copyright (C) 2018 Sony Semiconductor Solutions Corp.
  Copyright (c) 2017 Sony Corporation  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdbool.h>
#include <stdio.h>
#include <sdk/config.h>
#include <arch/board/board.h>
#include <arch/chip/pin.h>
#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <common/up_arch.h>
#include <chip/cxd5602_memorymap.h>
#include <chip/cxd5602_topreg.h>

#include <Arduino.h>
#include "utility.h"
#include "wiring.h"
#include "wiring_private.h"

#ifndef CONFIG_CXD56_GPIO_IRQ
# error Please enable GPIO interrupt support in NuttX
#endif // CONFIG_CXD56_GPIO_IRQ

static irqstate_t s_irq_flags;

#define INTC_EN(n) (CXD56_INTC_BASE + 0x10 + (((n) >> 5) << 2))

static bool irq_enabled(int irq)
{
    assert(irq >= CXD56_IRQ_EXTINT);

    noInterrupts();
    irq -= CXD56_IRQ_EXTINT;
    uint32_t bit = 1 << (irq & 0x1f);
    uint32_t regval = getreg32(INTC_EN(irq));
    bool enabled = ((regval & bit) ? true : false);
    interrupts();

    return enabled;
}

static void attach_interrupt(uint8_t pin, void (*isr)(void), int mode)
{
    int  _mode;
    bool filter = true; // always enable noise filter

    switch (mode) {
    case LOW:
        _mode = INT_LOW_LEVEL;
        break;
    case HIGH:
        _mode = INT_HIGH_LEVEL;
        break;
    case CHANGE:
        _mode = INT_BOTH_EDGE;
        break;
    case RISING:
        _mode = INT_RISING_EDGE;
        break;
    case FALLING:
        _mode = INT_FALLING_EDGE;
        break;
    default:
        printf("ERROR: unknown interrupt mode [%d]\n", mode);
        return;
    }

    int irq = board_gpio_intconfig(pin, _mode, filter, (xcpt_t)isr);
    if (irq < 0) {
        printf("ERROR: Out of interrupt resources\n");
        return;
    }

    /* wait RTC few cycles before the interrupt is enabled for noise filter. */
    delay(1);
    board_gpio_int(pin, true);
}

static void detach_interrupt(uint8_t pin)
{
    int irq = board_gpio_int(pin, false);
    if (irq < 0) {
        printf("ERROR: Invalid pin [%d]\n", pin);
        return;
    }
    board_gpio_intconfig(pin, 0, false, NULL);
}

extern "C" {
void interrupts(void)
{
    leave_critical_section(s_irq_flags);
}

void noInterrupts(void)
{
    s_irq_flags = enter_critical_section();
}

uint16_t irq_save(uint16_t mask)
{
    uint16_t flags = 0;
    for (int i = CXD56_IRQ_EXDEVICE_0; i <= CXD56_IRQ_EXDEVICE_11; ++i) {
        int b = i - CXD56_IRQ_EXDEVICE_0;
        if (irq_enabled(i)) {
            bitSet(flags, b);
            if (bitRead(mask, b))
                up_disable_irq(i);
        }
    }

    return flags;
}

void irq_restore(uint16_t flags)
{
    for (int i = CXD56_IRQ_EXDEVICE_0; i <= CXD56_IRQ_EXDEVICE_11; ++i) {
        if (bitRead(flags, i - CXD56_IRQ_EXDEVICE_0))
            up_enable_irq(i);
    }
}
} // extern "C"

void attachInterrupt(uint8_t interrupt, void (*isr)(void), int mode)
{
    uint8_t _pin = pin_convert(interrupt);
    if (_pin == PIN_NOT_ASSIGNED)
        return;
    attach_interrupt(_pin, isr, mode);
}

void detachInterrupt(uint8_t interrupt)
{
    uint8_t _pin = pin_convert(interrupt);
    if (_pin == PIN_NOT_ASSIGNED)
        return;
    detach_interrupt(_pin);
}

// Timer Interrupt

static struct timer_int_s {
    int fd;
    unsigned int (*isr)(void);
} s_timer_int = { -1, NULL };

static bool timer_handler(unsigned int *next_interval_us, void *arg)
{
    unuse(arg);

    unsigned int next;

    if (!s_timer_int.isr) {
        return false;
    }

    next = s_timer_int.isr();
    if (next) {
        *next_interval_us = next;
        return true;
    } else {
        return false;
    }
}

void attachTimerInterrupt(unsigned int (*isr)(void), unsigned int us)
{
    int ret;
    int fd;

    if (s_timer_int.fd == -1) {
        ret = util_open_timer("/dev/timer0", &fd);
        if (ret) {
            return;
        }

        s_timer_int.fd = fd;
    }

    s_timer_int.isr = isr;
    util_start_timer(s_timer_int.fd, us, timer_handler);
}

void detachTimerInterrupt(void)
{
    util_stop_timer(s_timer_int.fd);
    util_close_timer(s_timer_int.fd);
    s_timer_int.fd = -1;
}
