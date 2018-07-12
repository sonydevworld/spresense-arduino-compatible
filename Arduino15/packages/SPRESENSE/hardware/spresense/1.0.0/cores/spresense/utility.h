/*
  utility.h - Utilities for the Sparduino SDK
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

#ifndef Utility_h
#define Utility_h

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdbool.h>
#include <nuttx/config.h>
#include <sdk/config.h>
#ifdef CONFIG_TIMER
#include <nuttx/timers/timer.h>
#include <cxd56_timer.h>
#else
#error Please enable TIMER in nuttx
#endif // CONFIG_TIMER

/* logger */
#ifndef LOG_PREFIX
#define LOG_PREFIX  "????"
#endif // LOG_PREFIX

#include <stdio.h>
#define LOG_C(fmt...)   printf("<C>" "[" LOG_PREFIX "]" fmt)
#define LOG_E(fmt...)   printf("<E>" "[" LOG_PREFIX "]" fmt)
#define LOG_W(fmt...)   printf("<W>" "[" LOG_PREFIX "]" fmt)
#define LOG_I(fmt...)   printf("<I>" "[" LOG_PREFIX "]" fmt)
#define LOG_D(fmt...)   printf("<D>" "[" LOG_PREFIX "]" fmt)

/* helper */
#define unuse(x)            (void)(x)
#define arrayLength(a)      (sizeof(a) / sizeof((a)[0]))
#define arrayForEach(a, i)  for (unsigned int i = 0; i < arrayLength(a); ++i)

/* timer */
int util_open_timer(const char* dev_name, int* fd);
int util_close_timer(int fd);
int util_start_timer(int fd, unsigned long timeout /*us*/, tccb_t handler);
int util_stop_timer(int fd);
bool util_timer_is_running(int fd);
uint32_t util_get_time_out(int fd);         // return actual time out value in us
uint32_t util_get_time_left(int fd);        // return timer left time value in us
uint32_t util_get_time_collapsed(int fd);   // return timer passed time value in us

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // Utility_h
