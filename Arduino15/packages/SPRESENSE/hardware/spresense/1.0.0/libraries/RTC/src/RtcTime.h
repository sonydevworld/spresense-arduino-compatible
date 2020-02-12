/*
 *  RtcTime.h - Spresense Arduino RTC library
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

#ifndef __RTCTIME_H__
#define __RTCTIME_H__

/**
 * @defgroup rtc RTC Library API
 * @{
 */

/**
 * @class RtcTime
 * @brief RTC time definitions
 *
 * @details This is the time class defined to access to the RtcClass.
 */
class RtcTime
{
public:
  /**
   * @brief Create RtcTime object
   */
  RtcTime(uint32_t sec = 0, long nsec = 0);
  RtcTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0);
  RtcTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0, long nsec = 0);
  RtcTime(const char* date, const char* time);

  /**
   * @brief Getter APIs of RtcTime
   */
  uint32_t unixtime() const { return _sec; }
  long nsec() const { return _nsec; }
  int year() const { return _year; }
  int month() const { return _month; }
  int day() const { return _day; }
  int hour() const { return _hour; }
  int minute() const { return _minute; }
  int second() const { return _second; }

  /**
   * @brief Setter APIs of RtcTime
   */
  void unixtime(uint32_t sec);
  void nsec(long nsec);
  void year(int year);
  void month(int month);
  void day(int day);
  void hour(int hour);
  void minute(int minute);
  void second(int second);

  /**
   * @brief operator APIs to compare and calculate with RtcTime
   */
  bool operator == (const RtcTime& other) const {
    return (_sec == other._sec);
  }

  bool operator != (const RtcTime& other) const {
    return !(*this == other);
  }

  void operator += (uint32_t seconds) {
    RtcTime after = RtcTime(unixtime() + seconds);
    *this = after;
  }

  void operator -= (uint32_t seconds) {
    RtcTime before = RtcTime(unixtime() - seconds);
    *this = before;
  }

  operator uint32_t() const {
    return unixtime();
  }

private:
  uint32_t _sec;    /* UNIX time in Seconds (time_t) */
  long     _nsec;   /* Nanoseconds */
  int      _year;   /* Years */
  int      _month;  /* Month (1-12) */
  int      _day;    /* Day of the month (1-31) */
  int      _hour;   /* Hours (0-23) */
  int      _minute; /* Minutes (0-59) */
  int      _second; /* Seconds (0-61, allows for leap seconds) */
  void update(int year, int month, int day, int hour, int minute, int second);
  void update();
};

/** @} rtc */

#endif // __RTCETIME_H__
