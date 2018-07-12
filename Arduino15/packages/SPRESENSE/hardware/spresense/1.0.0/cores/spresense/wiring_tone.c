/*
  wiring_tone.c - Tone functions
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
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <nuttx/config.h>
#include <sdk/config.h>
#include <common/up_arch.h>
#include <Arduino.h>
#include "utility.h"
#include "wiring_private.h"

#define TONE_TIMER_ID           CXD56_TIMER0
#define TIMER_FD_INVALID        (-1)
#define TONE_TIMER_DEV_NAME     "/dev/timer0"

typedef struct {
    uint8_t in_use:1;
    uint8_t pin;
    uint8_t infinite:1;
    uint8_t timer_id;
    uint32_t pin_addr;
    int timer_fd;
    unsigned long duration; // us
    unsigned long interval; // us
} tone_ctx_t;

static tone_ctx_t s_ctx = {
    .in_use = false,
    .pin = PIN_NOT_ASSIGNED,
    .infinite = false,
    .timer_id = TONE_TIMER_ID,
    .timer_fd = TIMER_FD_INVALID,
    .duration = 0,
    .interval = 0
};

static bool timer_handler(FAR uint32_t *next_interval_us, FAR void *arg)
{
    //printf("timer_handler %llu\n", micros());
    static bool ret;
    static uint32_t val;

    unuse(arg);

    if (!s_ctx.infinite) {
        assert(s_ctx.duration >= s_ctx.interval);
        s_ctx.duration -= s_ctx.interval;
        if (s_ctx.duration > 0 && s_ctx.duration < s_ctx.interval) {
            s_ctx.interval = s_ctx.duration;
            *next_interval_us = s_ctx.interval;
        }
    }

    if (s_ctx.duration || s_ctx.infinite) {
        val = getreg32(s_ctx.pin_addr);
        if (bitRead(val, GPIO_OUTPUT_SHIFT))
            bitClear(val, GPIO_OUTPUT_SHIFT);
        else
            bitSet(val, GPIO_OUTPUT_SHIFT);
        putreg32(val, s_ctx.pin_addr);
        ret = true;
        // digital Read/Write slows down handle speed
        //int val = digitalRead(s_ctx.pin);
        //digitalWrite(s_ctx.pin, (val + 1) % 2);
    }
    else {
        noTone(s_ctx.pin);
        ret = false;
    }

    return ret;
}

static int start_timer(unsigned long timeout /*us*/)
{
    return util_start_timer(s_ctx.timer_fd, timeout, timer_handler);
}

static int stop_timer(void)
{
    return util_stop_timer(s_ctx.timer_fd);
}

static int tone_setup(void)
{
    return s_ctx.timer_fd < 0 ? util_open_timer(TONE_TIMER_DEV_NAME, &s_ctx.timer_fd) : OK;
}

static void tone_teardown(void)
{
    if (s_ctx.timer_fd >= 0)
        (void) util_close_timer(s_ctx.timer_fd);
    s_ctx.timer_fd = TIMER_FD_INVALID;
}

static int tone_begin(uint8_t pin, unsigned int frequency, unsigned long duration)
{
    if (tone_setup() != OK)
        return ERROR;

    s_ctx.pin = pin;
    s_ctx.infinite = (duration == 0);
    s_ctx.duration = duration * 1000; // convert to us
    s_ctx.interval = 1000000L / frequency / 2;
    s_ctx.pin_addr = get_gpio_regaddr((uint32_t)pin_convert(pin));

    if (!s_ctx.infinite && s_ctx.duration < s_ctx.interval) {
        s_ctx.interval = s_ctx.duration;
    }

    if (!s_ctx.in_use) {
        s_ctx.in_use = true;
        pinMode(s_ctx.pin, OUTPUT);
    }
    else
        (void) stop_timer();

    digitalWrite(s_ctx.pin, HIGH);
    return start_timer(s_ctx.interval);
}

static void tone_end(void)
{
    (void) stop_timer();
    tone_teardown();
    digitalWrite(s_ctx.pin, LOW);

    s_ctx.pin_addr = 0;
    s_ctx.pin = PIN_NOT_ASSIGNED;
    s_ctx.infinite = false;
    s_ctx.interval = 0;
    s_ctx.duration = 0;
    s_ctx.in_use = false;
}

void tone(uint8_t pin, unsigned int frequency, unsigned long duration)
{
    // sanity check
    if (frequency == 0 || pin_convert(pin) == PIN_NOT_ASSIGNED) return;

    // state check
    if (s_ctx.in_use && s_ctx.pin != pin) return;

    int ret = tone_begin(pin, frequency, duration);
    if (ret != OK)
        printf("tone failed at tone_begin the errno is %d\n",ret);
}

void noTone(uint8_t pin)
{
    if (!s_ctx.in_use || s_ctx.pin != pin) return;
    tone_end();
}
