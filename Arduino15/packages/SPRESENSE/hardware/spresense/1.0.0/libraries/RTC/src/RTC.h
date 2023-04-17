/*
 *  RTC.h - Spresense Arduino RTC library
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

#ifndef __RTC_H__
#define __RTC_H__

/**
 * @file RTC.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino RTC library
 *
 * @details The RTC library allows for getting/setting from/to the date and
 *          time or the alarm of Real-Time Clock.
 */

/**
 * @defgroup rtc RTC Library API
 * @brief API for using Real Time Clock library API
 * @{
 */

#include "RtcTime.h"

/**
 * @class RtcClass
 * @brief This is the interface to the RTC Hardware.
 *
 */
class RtcClass
{
public:
  /**
   * @brief Create RtcClass object
   */
  RtcClass() : _fd(-1), _pid(-1) {}

  /**
   * @brief Initialize the RTC library
   * @details When RTC library is used, this API must be called at first.
   *          This API will wait until RTC hardware is available.
   */
  void begin();

  /**
   * @brief Finalize the RTC library
   */
  void end();

  /**
   * @brief Set RTC time
   * @param [in] rtc a object of RtcTime to set
   */
  void setTime(RtcTime &rtc);

  /**
   * @brief Get RTC time
   * @return a object of the current RtcTime
   */
  RtcTime getTime();

#ifndef SUBCORE
  /**
   * @brief Set RTC alarm time
   * @param [in] rtc a object of RtcTime to set the alarm
   */
  void setAlarm(RtcTime &rtc);

  /**
   * @brief Set RTC alarm time after the specified seconds
   * @param [in] seconds to set the alarm
   */
  void setAlarmSeconds(uint32_t seconds);

  /**
   * @brief Cancel RTC alarm time
   */
  void cancelAlarm();

  /**
   * @brief Attach the alarm handler
   * @param [in] isr the alarm handler which is executed on the task context.
   */
  void attachAlarm(void (*isr)(void));

  /**
   * @brief Detach the alarm handler
   */
  void detachAlarm();

#endif /* !SUBCORE */

private:
  int _fd;
  pthread_t _pid;

};

extern RtcClass RTC;

/** @} rtc */

#endif // __RTC_H__
