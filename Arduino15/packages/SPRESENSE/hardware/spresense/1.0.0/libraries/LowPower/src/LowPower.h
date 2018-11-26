/*
 *  LowPower.h - Spresense Arduino Low Power library
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

#ifndef _LOW_POWER_H_
#define _LOW_POWER_H_

/**
 * @file LowPower.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino Low Power library
 *
 * @details The Low Power library can manage the low power states of Spresense.
 */

#include <Arduino.h>

typedef enum {
  POR_DEADBATT    = 0,  /** Power On Reset in DeadBattery state */
  WDT_REBOOT      = 1,  /** System WDT expired or Self Reboot */
  WDT_RESET       = 2,  /** Chip WDT expired */
  DEEP_WKUPL      = 3,  /** In DeepSleep state, Detected WKUPL signal */
  DEEP_WKUPS      = 4,  /** In DeepSleep state, Detected WKUPS signal */
  DEEP_RTC        = 5,  /** In DeepSleep state, RTC Alarm expired */
  DEEP_USB_ATTACH = 6,  /** In DeepSleep state, USB Connected */
  DEEP_OTHERS     = 7,  /** In DeepSleep state, Reserved others cause occurred */
  COLD_SCU_INT    = 8,  /** In ColdSleep state, Detected SCU Interrupt */
  COLD_RTC_ALM0   = 9,  /** In ColdSleep state, RTC Alarm0 expired */
  COLD_RTC_ALM1   = 10, /** In ColdSleep state, RTC Alarm1 expired */
  COLD_RTC_ALM2   = 11, /** In ColdSleep state, RTC Alarm2 expired */
  COLD_RTC_ALMERR = 12, /** In ColdSleep state, RTC Alarm Error occurred */
  COLD_GPIO_IRQ36 = 16, /** In ColdSleep state, Detected GPIO IRQ 36 */
  COLD_GPIO_IRQ37 = 17, /** In ColdSleep state, Detected GPIO IRQ 37 */
  COLD_GPIO_IRQ38 = 18, /** In ColdSleep state, Detected GPIO IRQ 38 */
  COLD_GPIO_IRQ39 = 19, /** In ColdSleep state, Detected GPIO IRQ 39 */
  COLD_GPIO_IRQ40 = 20, /** In ColdSleep state, Detected GPIO IRQ 40 */
  COLD_GPIO_IRQ41 = 21, /** In ColdSleep state, Detected GPIO IRQ 41 */
  COLD_GPIO_IRQ42 = 22, /** In ColdSleep state, Detected GPIO IRQ 42 */
  COLD_GPIO_IRQ43 = 23, /** In ColdSleep state, Detected GPIO IRQ 43 */
  COLD_GPIO_IRQ44 = 24, /** In ColdSleep state, Detected GPIO IRQ 44 */
  COLD_GPIO_IRQ45 = 25, /** In ColdSleep state, Detected GPIO IRQ 45 */
  COLD_GPIO_IRQ46 = 26, /** In ColdSleep state, Detected GPIO IRQ 46 */
  COLD_GPIO_IRQ47 = 27, /** In ColdSleep state, Detected GPIO IRQ 47 */
  COLD_SEN_INT    = 28, /** In ColdSleep state, Detected SEN_INT Interrupt */
  COLD_PMIC_INT   = 29, /** In ColdSleep state, Detected PMIC Interrupt */
  COLD_USB_DETACH = 30, /** In ColdSleep state, USB Disconnected */
  COLD_USB_ATTACH = 31, /** In ColdSleep state, USB Connected */
  POR_NORMAL      = 32, /** Power On Reset as battery attached */
} bootcause_e;

/**
 * @class LowPowerClass
 * @brief This provides the features fo the power saving
 *
 */
class LowPowerClass
{
public:
  /**
   * @brief Initialize the Low Power library
   * @details This initializes RTC library, because this library uses the RTC library.
   */
  void begin();

  /**
   * @brief Finalize the Low Power library
   */
  void end();

  /**
   * @brief Sleep (yield) this thread
   * @param [in] seconds - the sleep period
   * @details Just call the system call of sleep()
   */
  void sleep(uint32_t seconds);

  /**
   * @brief Enter the cold sleep state
   */
  void coldSleep();

  /**
   * @brief Enter the cold sleep state during the specified seconds
   * @param [in] seconds - the period in cold sleep
   */
  void coldSleep(uint32_t seconds);

  /**
   * @brief Enter the deep sleep state
   */
  void deepSleep();

  /**
   * @brief Enter the deep sleep state during the specified seconds
   * @param [in] seconds - the period in deep sleep
   */
  void deepSleep(uint32_t seconds);

  /**
   * @brief Reboot the system
   */
  void reboot();

  /**
   * @brief Get the boot cause
   * @return a boot cause
   */
  bootcause_e bootCause();

  /**
   * @brief Check if the specified cause is permitted or not
   * @return true if permitted, otherwise return false
   */
  bool isEnabledBootCause(bootcause_e bc);

  /**
   * @brief Enable the specified cause as the boot cause
   * @param [in] bc - a boot cause
   */
  void enableBootCause(bootcause_e bc);

  /**
   * @brief Disable the specified cause as the boot cause
   * @param [in] bc - a boot cause
   */
  void disableBootCause(bootcause_e bc);

  /**
   * @brief Get a wakeup pin number from the boot cause
   * @param [in] bc - a boot cause
   * @return a pin number.
   */
  uint8_t getWakeupPin(bootcause_e bc);

};

extern LowPowerClass LowPower;

#endif
