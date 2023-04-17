/*
 *  RTC.cpp - Spresense Arduino RTC library
 *  Copyright 2018 Sony Semiconductor Solutions Corporation
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

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <errno.h>
#include <nuttx/arch.h>
#include <nuttx/timers/rtc.h>

#include <Arduino.h>
#include "RTC.h"

#define ALARM_DEVPATH "/dev/rtc0"
#define ALARM_SIGNO 1

#define ERRMSG(format, ...) printf("ERROR: " format, ##__VA_ARGS__)

#ifndef SUBCORE
static void (*g_isr)(void) = NULL;
static int g_alarm = 0;

static void alarm_handler(int signo, FAR siginfo_t *info, FAR void *ucontext)
{
  (void)signo;
  (void)info;
  (void)ucontext;
  /* Set alarm flag */
  g_alarm = 1;
}

static void *alarm_daemon(void *arg)
{
  struct sigaction act;
  sigset_t set;
  int ret;

  (void)arg;

  /* Make sure that the alarm signal is unmasked */

  sigfillset(&set);
  sigdelset(&set, ALARM_SIGNO);
  ret = sigprocmask(SIG_SETMASK, &set, NULL);
  assert(ret == OK);

  /* Register alarm signal handler */

  act.sa_sigaction = alarm_handler;
  act.sa_flags     = SA_SIGINFO;

  sigfillset(&act.sa_mask);
  sigdelset(&act.sa_mask, ALARM_SIGNO);

  ret = sigaction(ALARM_SIGNO, &act, NULL);
  assert(ret == OK);

  /* Now loop forever, waiting for alarm signals */

  for (; ; )
    {
      ret = sigwaitinfo(&set, NULL);
      if ((ret < 0) && (errno != EINTR)) {
        ERRMSG("%s() (errno=%d)\n", __FUNCTION__, errno);
      }
      if (g_alarm && g_isr) {
        g_isr();
        /* Clear alarm flag */
        g_alarm = 0;
      }
    }

  return NULL;
}
#endif /* !SUBCORE */

void RtcClass::begin()
{
#ifndef SUBCORE
  /* NOTE: After the driver has been opened, a task should be created,
   * because the created task shares the file descriptor (fd) of driver.
   */

  if (_fd < 0) {
    _fd = open(ALARM_DEVPATH, O_WRONLY);
    assert(_fd > 0);
  }

  if (_pid < 0) {
    struct sched_param param;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    attr.stacksize = 2048;
    param.sched_priority = 120;
    pthread_attr_setschedparam(&attr, &param);

    pthread_create(&_pid, &attr, (pthread_startroutine_t)alarm_daemon, NULL);
    pthread_setname_np(_pid, "alarm_daemon");
    assert (_pid > 0);
  }
#endif /* !SUBCORE */

  /* Wait until RTC is available */

  while (g_rtc_enabled == false);
}

void RtcClass::end()
{
#ifndef SUBCORE
  if (_pid > 0) {
    pthread_cancel(_pid);
    _pid = -1;
  }

  if (_fd > 0) {
    close(_fd);
    _fd = -1;
  }
#endif /* !SUBCORE */
}

void RtcClass::setTime(RtcTime &tim)
{
  struct timespec ts;

  ts.tv_sec  = tim.unixtime();
  ts.tv_nsec = tim.nsec();

  clock_settime(CLOCK_REALTIME, &ts);
  return;
}

RtcTime RtcClass::getTime()
{
  struct timespec ts;

  int ret = clock_gettime(CLOCK_REALTIME, &ts);

  if (ret) {
    /* if error occurs, returns the date of 1970/1/1 */
    ts.tv_sec = ts.tv_nsec = 0;
    ERRMSG("%s() (errno=%d)\n", __FUNCTION__, errno);
  }
  return RtcTime(ts.tv_sec, ts.tv_nsec);
}

#ifndef SUBCORE
void RtcClass::setAlarm(RtcTime &tim)
{
  int ret;
  struct rtc_setalarm_s setalm;

  if ((_fd < 0) || (_pid < 0)) {
    ERRMSG("Please call RTC.begin() in advance\n");
    return;
  }

  /* Set the alarm of the absolute time */
  setalm.id           = 0;
  setalm.pid          = _pid;
  setalm.time.tm_sec  = tim.second();
  setalm.time.tm_min  = tim.minute();
  setalm.time.tm_hour = tim.hour();
  setalm.time.tm_mday = tim.day();
  setalm.time.tm_mon  = tim.month() - 1;
  setalm.time.tm_year = tim.year() - 1900;

  setalm.event.sigev_notify = SIGEV_SIGNAL;
  setalm.event.sigev_signo  = ALARM_SIGNO;
  setalm.event.sigev_value.sival_int = 0;

  ret = ioctl(_fd, RTC_SET_ALARM, (unsigned long)((uintptr_t)&setalm));
  if (ret < 0) {
    ERRMSG("%s() (errno=%d)\n", __FUNCTION__, errno);
  }
}

void RtcClass::setAlarmSeconds(uint32_t seconds)
{
  int ret;
  struct rtc_setrelative_s setrel;

  if ((_fd < 0) || (_pid < 0)) {
    ERRMSG("Please call RTC.begin() in advance\n");
    return;
  }

  /* Set the alarm expired after the specified seconds */
  setrel.id      = 0;
  setrel.pid     = _pid;
  setrel.reltime = (time_t)seconds;

  setrel.event.sigev_notify = SIGEV_SIGNAL;
  setrel.event.sigev_signo  = ALARM_SIGNO;
  setrel.event.sigev_value.sival_int = 0;


  ret = ioctl(_fd, RTC_SET_RELATIVE, (unsigned long)((uintptr_t)&setrel));
  if (ret < 0) {
    ERRMSG("%s() (errno=%d)\n", __FUNCTION__, errno);
  }
}

void RtcClass::cancelAlarm()
{
  int ret;

  ret = ioctl(_fd, RTC_CANCEL_ALARM, 0);
  if (ret < 0) {
    ERRMSG("%s() (errno=%d)\n", __FUNCTION__, errno);
  }
}

void RtcClass::attachAlarm(void (*isr)(void))
{
  if ((_fd < 0) || (_pid < 0)) {
    ERRMSG("Please call RTC.begin() in advance\n");
  }
  g_isr = isr;
}

void RtcClass::detachAlarm()
{
  g_isr = NULL;
}
#endif /* !SUBCORE */

RtcClass RTC;
