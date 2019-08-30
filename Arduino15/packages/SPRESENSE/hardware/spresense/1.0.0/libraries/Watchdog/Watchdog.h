/*
 *  Watchdog.h - Spresense Arduino Watchdog library 
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

#ifndef Watchdog_h
#define Watchdog_h

/**
 * @file Watchdog.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino Watchdog Library 
 * 
 * @details This is a library about watchdog for user application, and
 *          application can check what application is alive and reset if freezed.
 */

/**
 * @defgroup watchdog Watchdog Library API
 * @brief API for using Watchdog
 * @{
 */

/*
  This header file maybe inclued in plain C file.
  To avoid compiling error all C++ stuff should be ignored
 */

#include <sdk/config.h>
#include <nuttx/config.h>
#include <nuttx/timers/watchdog.h>
#include <Arduino.h>


/*
  For debug kernel
 */
#if BRD_DEBUG
#define watchdog_printf(...) printf(__VA_ARGS__)
#else
#define watchdog_printf(x...)
#endif

/**
 * @class WatchdogClass
 * @brief Watchdog controller
 *
 * @details You can reset your application when application freezed
 *          by operating WatchdogClass objects instantiated in your application.
 */
class WatchdogClass {
public:
    /**
     * @brief Create WatchdogClass object
     *
     * @details Application need to take instance for using watchdog.
     */
    WatchdogClass(void);

    /**
     * @brief Initialize the Watchdog library
     *
     * @details Open the hardware watchdog device file and keep it.
     */
    void begin(void);

    /**
     * @brief Disable the Watchdog
     *
     * @details Stop the hardware watchdog and close device file for release it.
     */
    void end(void);

    /**
     * @brief Start to check timer for bite watchdog
     *
     * @details Start to count and check time for bite watchdog. And if it expire timeout value,
     *          device will reboot by hardware trigger.
     *
     * @param [in] timeout Timeout value in milliseconds for bite a watchdog (1 ~ 40000)
     */
    void start(uint32_t timeout);

    /**
     * @brief Stop to check timer for avoid bite watchdog
     *
     * @details Stop to count and check timer. After call this function, device will not reboot
     *          by watchdog.
     */
    void stop(void);

    /**
     * @brief Kick to watchdog for notify keep alive
     *
     * @details Kick the dog to avoid bite a watchdog, it mean "Keep alive".
     * If expire a timeout, device will reboot by hardware trigger.
     */
    void kick(void);

    /**
     * @brief Get a remain time for bite watchdog.
     *
     * @details Get a remain time to expire timeout.
     *
     * @return Remain time in milliseconds >= 0
     */
    uint32_t timeleft(void);

private:
    int wd_fd;                  /**< File descriptor for use watchdog device file */
};

extern WatchdogClass Watchdog;

/** @} watchdog */

#endif // Watchdog_h
