/*
  time.c - Time API implementation file for the Sparduino SDK
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

#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <nuttx/config.h>
#include <sdk/config.h>
#include <nuttx/arch.h>
#include <cxd56_clock.h>
#include <Arduino.h>

#ifndef CONFIG_RTC
# error Please enable RTC in NuttX
#endif // CONFIG_RTC

#ifndef CONFIG_RTC_HIRES
# error Please enable RTC High Resolution in NuttX
#endif // CONFIG_RTC_HIRES

#ifndef CONFIG_CLOCK_MONOTONIC
# error Please enable monotonic clock in NuttX
#endif // CONFIG_CLOCK_MONOTONIC

#define DELAY_CORRECTION    (1228)

uint64_t millis(void)
{
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp)) {
        return 0;
    }

    return (((uint64_t)tp.tv_sec) * 1000 + tp.tv_nsec / 1000000);
}

uint64_t micros(void)
{
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp)) {
        return 0;
    }

    return (((uint64_t)tp.tv_sec) * 1000000 + tp.tv_nsec / 1000);
}

void delay(unsigned long ms)
{
    if (ms)
        up_mdelay(ms);
        //usleep(ms * 1000);
}

void delayMicroseconds(unsigned int us)
{
    // up_udelay is not as accurate as the following implementation
    //if (us) up_udelay(us);

    if (us) {
        unsigned long ticks = microsecondsToClockCycles(us);
        if (ticks < DELAY_CORRECTION) return; // delay time already used in calculation

        ticks -= DELAY_CORRECTION;
        ticks >>= 2;
        // following loop takes 4 cycles
        do {
            __asm__ __volatile__("nop");
        } while(--ticks);
    }
}

unsigned long clockCyclesPerMicrosecond(void)
{
    return cxd56_get_cpu_baseclk() / 1000000L;
}
