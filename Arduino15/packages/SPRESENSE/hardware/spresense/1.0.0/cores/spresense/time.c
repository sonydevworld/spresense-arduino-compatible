/*
  time.c - Time API implementation file for the Spresense SDK
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
#include <cxd56_rtc.h>
#include <Arduino.h>

#ifndef CONFIG_RTC
# error Please enable RTC in NuttX
#endif // CONFIG_RTC

#ifndef CONFIG_RTC_HIRES
# error Please enable RTC High Resolution in NuttX
#endif // CONFIG_RTC_HIRES

#define DELAY_CORRECTION    (700)
#define DELAY_INTERVAL      (50)

uint64_t millis(void)
{
    uint64_t count;

    /* Wait until RTC is available */
    while (g_rtc_enabled == false);

    count = cxd56_rtc_count();

    /* The count represents the power-on time,
     * so overflow does not actually occur.
     */

    return (count * MSEC_PER_SEC + CONFIG_RTC_FREQUENCY / 2) / CONFIG_RTC_FREQUENCY;
}

uint64_t micros(void)
{
    uint64_t count;

    /* Wait until RTC is available */
    while (g_rtc_enabled == false);

    count = cxd56_rtc_count();

    /* The count represents the power-on time,
     * so overflow does not actually occur.
     */

    return (count * USEC_PER_SEC + CONFIG_RTC_FREQUENCY / 2) / CONFIG_RTC_FREQUENCY;
}

void delayMicroseconds(unsigned int us)
{
    // up_udelay is not as accurate as the following implementation
    //if (us) up_udelay(us);

    if (us) {
        unsigned long long ticks = microsecondsToClockCycles(us);
        if (ticks < DELAY_CORRECTION) return; // delay time already used in calculation

        ticks -= DELAY_CORRECTION;
        ticks /= 6;
        // following loop takes 6 cycles
        do {
            __asm__ __volatile__("nop");
        } while(--ticks);
    }
}

void delay(unsigned long ms)
{
    if (ms) {
        while (DELAY_INTERVAL < ms) {
            delayMicroseconds(DELAY_INTERVAL * 1000);
            ms -= DELAY_INTERVAL;
        }
        delayMicroseconds(ms * 1000);
    }
}

unsigned long clockCyclesPerMicrosecond(void)
{
    return cxd56_get_cpu_baseclk() / 1000000L;
}
