/*
 *  RtcTime.cpp - Spresense Arduino RTC library
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

#include <Arduino.h>
#include "RtcTime.h"
#include <time.h>

RtcTime::RtcTime(uint32_t sec, long nsec)
     : _sec(sec), _nsec(nsec)
{
  update();
}

RtcTime::RtcTime(int year, int month, int day, int hour, int minute, int second, long nsec)
     : _nsec(nsec),
       _year(year), _month(month), _day(day),
       _hour(hour), _minute(minute), _second(second)
{
  update(_year, _month, _day, _hour, _minute, _second);
};

RtcTime::RtcTime(const char* date, const char* time)
{
  String strbuf;

  // year
  strbuf = String(date).substring(6);
  _year = strbuf.toInt();
  // month
  strbuf = String(date).substring(0, 3);
  if      (strbuf == "Jan") _month =  1;
  else if (strbuf == "Feb") _month =  2;
  else if (strbuf == "Mar") _month =  3;
  else if (strbuf == "Apr") _month =  4;
  else if (strbuf == "May") _month =  5;
  else if (strbuf == "Jun") _month =  6;
  else if (strbuf == "Jul") _month =  7;
  else if (strbuf == "Aug") _month =  8;
  else if (strbuf == "Sep") _month =  9;
  else if (strbuf == "Oct") _month = 10;
  else if (strbuf == "Nov") _month = 11;
  else if (strbuf == "Dec") _month = 12;
  // day
  strbuf = String(date).substring(4, 6);
  _day = strbuf.toInt();
  // hour
  strbuf = String(time).substring(0, 2);
  _hour = strbuf.toInt();
  // minute
  strbuf = String(time).substring(3, 5);
  _minute = strbuf.toInt();
  // second
  strbuf = String(time).substring(6, 8);
  _second = strbuf.toInt();

  _nsec = 0;
  update(_year, _month, _day, _hour, _minute, _second);
}

void RtcTime::unixtime(uint32_t sec)
{
  _sec = sec;
  update();
}

void RtcTime::nsec(long nsec)
{
  _nsec = nsec;
  update(_year, _month, _day, _hour, _minute, _second);
}

void RtcTime::year(int year)
{
  _year = year;
  update(_year, _month, _day, _hour, _minute, _second);
}

void RtcTime::month(int month)
{
  _month = month;
  update(_year, _month, _day, _hour, _minute, _second);
}

void RtcTime::day(int day)
{
  _day = day;
  update(_year, _month, _day, _hour, _minute, _second);
}

void RtcTime::hour(int hour)
{
  _hour = hour;
  update(_year, _month, _day, _hour, _minute, _second);
}

void RtcTime::minute(int minute)
{
  _minute = minute;
  update(_year, _month, _day, _hour, _minute, _second);
}

void RtcTime::second(int second)
{
  _second = second;
  update(_year, _month, _day, _hour, _minute, _second);
}

void RtcTime::update(int year, int month, int day, int hour, int minute, int second)
{
  struct tm tm;
  tm.tm_sec  = second;      /* Seconds (0-61, allows for leap seconds) */
  tm.tm_min  = minute;      /* Minutes (0-59) */
  tm.tm_hour = hour;        /* Hours (0-23) */
  tm.tm_mday = day;         /* Day of the month (1-31) */
  tm.tm_mon  = month - 1;   /* Month (0-11) */
  tm.tm_year = year - 1900; /* Years since 1900 */
  _sec = mktime(&tm);

  update();
}

void RtcTime::update()
{
  struct tm tm;

  if (_nsec >= 1000000000L) {
    _sec += _nsec / 1000000000L;
    _nsec %= 1000000000L;
  }

  gmtime_r(&_sec, &tm);
  _second = tm.tm_sec;
  _minute = tm.tm_min;
  _hour   = tm.tm_hour;
  _day    = tm.tm_mday;
  _month  = tm.tm_mon + 1;
  _year   = tm.tm_year + 1900;
}

