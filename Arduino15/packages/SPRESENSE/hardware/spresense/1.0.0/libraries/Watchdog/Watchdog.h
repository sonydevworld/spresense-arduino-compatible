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
 * @details It is a library for using watchdog for user program, with the 
 *          Spresense as the master device. 
 */

/*
  This header file maybe inclued in plain C file.
  To avoid compiling error all C++ stuff should be ignored
 */

#include <sdk/config.h>
#include <nuttx/config.h>
#include <nuttx/timers/watchdog.h>
#include <Arduino.h>

#define WATCHDOG_DEVPATH "/dev/watchdog0"


/**
 * @class WatchdogClass
 * @brief Watchdog controller
 *
 * @details You can control SPI comunication by operating WatchdogClass objects 
 *          instantiated in your app.
 */
class WatchdogClass {
public:
    /**
     * @brief Create SPIClass object
     * 
     * @param [in] port The default port is 4. You can control SPI4 using object SPI\n
     *                  e.g. SPI.begin();\n SPI5 is also supported. You can control
     *                  SPI5 using object SPI5\n e.g. SPI5.begin();
     */
    WatchdogClass(void);

    /**
     * @brief Initialize the SPI library
     */
    void begin(uint32_t);

    /**
     * @brief Disable the SPI bus
     */
    void end(void);

    /**
     * @brief Before using SPI.transfer() or asserting chip select pins,
     *        this function is used to gain exclusive access to the SPI bus
     *        and configure the correct settings.
     * 
     * @param [in] settings SPISettings object
     */
    void start(void);

    /**
     * @brief After performing a group of transfers and releasing the chip select
     *        signal, this function allows others to access the SPI bus
     */
    void stop(void);

    /**
     * @brief This function is deprecated.  New applications should use
     *        beginTransaction() to configure SPI settings.
     * 
     * @param [in] bitOrder Bit order
     */
    void kick(void);

private:
    int wd_fd;                  /**< SPI port number */
};

extern WatchdogClass Watchdog;

#endif // Watchdog_h
