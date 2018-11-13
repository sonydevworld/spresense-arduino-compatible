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
 * @brief Spresense Arduino Watchdog library 
 * 
 * @details It is a library for using watchdog for user program, and
 *          user can reset if program is freezed.
 */

/*
  This header file maybe inclued in plain C file.
  To avoid compiling error all C++ stuff should be ignored
 */

#include <sdk/config.h>
#include <nuttx/config.h>
#include <nuttx/timers/watchdog.h>
#include <Arduino.h>


/**
 * @class WatchdogClass
 * @brief Watchdog controller
 *
 * @details You can reset your application when application freezed
 *          by operating WatchdogClass objects instantiated in your app.
 */
class WatchdogClass {
public:
    /**
     * @brief Create WatchdogClass object
     */
    WatchdogClass(void);

    /**
     * @brief Initialize the Watchdog library
     *
     * @param [in] Timeout value for bite a watchdog
     */
    void begin(uint32_t);

    /**
     * @brief Disable the Watchdog
     */
    void end(void);

    /**
     * @brief Start to check timer for bite watchdog
     */
    void start(void);

    /**
     * @brief Stop to check timer for bite watchdog
     */
    void stop(void);

    /**
     * @brief Kick to avoid bite a watchdog
     */
    void kick(void);

private:
    int wd_fd;                  /**< File descriptor for use watchdog device file */
};

extern WatchdogClass Watchdog;

#endif // Watchdog_h
