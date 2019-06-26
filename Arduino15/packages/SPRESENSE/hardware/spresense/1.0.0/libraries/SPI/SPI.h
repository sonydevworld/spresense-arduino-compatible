/*
 *  SPI.h - Spresense Arduino SPI library 
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

#ifndef Spi_h
#define Spi_h

/**
 * @file SPI.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino SPI library 
 * 
 * @details It is a library for communicating with SPI devices, with the 
 *          Spresense as the master device. 
 */

/**
 * @defgroup spi SPI Library API
 * @brief API for using SPI
 * @{
 */

/*
  This header file maybe inclued in plain C file.
  To avoid compiling error all C++ stuff should be ignored
 */
// #ifdef __cplusplus

#include <sdk/config.h>
#include <nuttx/config.h>
#include <nuttx/spi/spi.h>
#include <Arduino.h>

// SPI_HAS_TRANSACTION means SPI has beginTransaction(), endTransaction(),
// usingInterrupt(), and SPISetting(clock, bitOrder, dataMode)
#define SPI_HAS_TRANSACTION 1

// SPI_HAS_NOTUSINGINTERRUPT means that SPI has notUsingInterrupt() method
#define SPI_HAS_NOTUSINGINTERRUPT 1

// SPI_ATOMIC_VERSION means that SPI has atomicity fixes and what version.
// This way when there is a bug fix you can check this define to alert users
// of your code if it uses better version of this library.
// This also implies everything that SPI_HAS_TRANSACTION as documented above is
// available too.
#define SPI_ATOMIC_VERSION 1

#define SPI_MODE0 SPIDEV_MODE0  /**< SPI mode 0 */
#define SPI_MODE1 SPIDEV_MODE1  /**< SPI mode 1 */
#define SPI_MODE2 SPIDEV_MODE2  /**< SPI mode 2 */
#define SPI_MODE3 SPIDEV_MODE3  /**< SPI mode 3 */

#define SPI_CLOCK_DIV2   2      /**< SPI Clock Divider 2 */
#define SPI_CLOCK_DIV4   4      /**< SPI Clock Divider 4 */
#define SPI_CLOCK_DIV8   8      /**< SPI Clock Divider 8 */
#define SPI_CLOCK_DIV16  16     /**< SPI Clock Divider 16 */
#define SPI_CLOCK_DIV32  32     /**< SPI Clock Divider 32 */
#define SPI_CLOCK_DIV64  64     /**< SPI Clock Divider 64 */
#define SPI_CLOCK_DIV128 128    /**< SPI Clock Divider 128 */

/**
 * @enum SpiInterruptMode
 * @brief Spi interrupt mode
 */
enum SpiInterruptMode {
    SPI_INT_MODE_NONE = 0,  /**< None */
    SPI_INT_MODE_MASK,      /**< Mask */
    SPI_INT_MODE_GLOBAL     /**< Global */
};

/**
 * @class SPISettings
 * @brief SPI settings
 *
 * @details Store SPI settings:
 *            - SPI clock frequency\n
 *              The default clock frequency is 4 MHz. The maximum frequency 
 *              supported is 20 MHz.
 *            - SPI bit order\n
 *              The default bit order is MSBFIRST.
 *            - SPI mode\n
 *              The default mode is SPI_MODE0.
 */
class SPISettings {
public:
    /**
     * @brief Construct a new SPISettings object
     * 
     * @param [in] clock Clock frequency
     * @param [in] bitOrder Bit order
     * @param [in] dataMode SPI mode
     */
    SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode)
        : clock_(clock), bit_order_(bitOrder), data_mode_(dataMode) { }

    /**
     * @brief Construct a new SPISettings object
     */
    SPISettings()
        : clock_(4000000), bit_order_(MSBFIRST), data_mode_(SPI_MODE0) { }

private:
    friend class SPIClass;
    uint32_t clock_;       /**< SPI clock frequency */
    uint8_t bit_order_;    /**< SPI bit order */
    uint8_t data_mode_;    /**< SPI mode */
};

/**
 * @class SPIClass
 * @brief SPI controller
 *
 * @details You can control SPI comunication by operating SPIClass objects 
 *          instantiated in your app.
 */
class SPIClass {
public:
    /**
     * @brief Create SPIClass object
     * 
     * @param [in] port The default port is 4. You can control SPI4 using object SPI\n
     *                  e.g. SPI.begin();\n SPI5 is also supported. You can control
     *                  SPI5 using object SPI5\n e.g. SPI5.begin();
     */
    SPIClass(int port);

    /**
     * @brief Initialize the SPI library
     */
    void begin(void);

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
    void beginTransaction(SPISettings settings);

    /**
     * @brief After performing a group of transfers and releasing the chip select
     *        signal, this function allows others to access the SPI bus
     */
    void endTransaction(void);

    /**
     * @brief This function is deprecated.  New applications should use
     *        beginTransaction() to configure SPI settings.
     * 
     * @param [in] bitOrder Bit order
     */
    void setBitOrder(uint8_t bitOrder);

    /**
     * @brief This function is deprecated.  New applications should use
     *        beginTransaction() to configure SPI settings.
     * 
     * @param [in] dataMode SPI mode
     */
    void setDataMode(uint8_t dataMode);

    /**
     * @brief This function is deprecated.  New applications should use
     *        beginTransaction() to configure SPI settings.
     * 
     * @param [in] clockDiv Clock divider
     */
    void setClockDivider(uint8_t clockDiv);

    /**
     * @brief Register interrupt with the SPI library
     *
     * @details If SPI is used from within an interrupt, this function registers
     *          that interrupt with the SPI library, so beginTransaction() can
     *          prevent conflicts.  The input interruptNumber is the number used
     *          with attachInterrupt.  If SPI is used from a different interrupt
     *          (eg, a timer), interruptNumber should be 255.
     * @note    The usingInterrupt and notUsingInterrupt functions should
     *          not to be called from ISR context or inside a transaction.
     * @see     For details see:\n
     *          <https://github.com/arduino/Arduino/pull/2381>\n
     *          <https://github.com/arduino/Arduino/pull/2449>
     * 
     * @param [in] interruptNumber Interrupt number
     */
    void usingInterrupt(uint8_t interruptNumber);

    /**
     * @brief Disable interrupt with the SPI library
     * 
     * @note  The usingInterrupt and notUsingInterrupt functions should
     *        not to be called from ISR context or inside a transaction.
     * @see   For details see:\n
     *        <https://github.com/arduino/Arduino/pull/2381>\n
     *        <https://github.com/arduino/Arduino/pull/2449>
     * 
     * @param [in] interruptNumber Interrupt number
     */
    void notUsingInterrupt(uint8_t interruptNumber);

    /**
     * @brief Write 8-bit data to the SPI bus and also receive 8-bit data
     * 
     * @param [in] data  8-bit data to send 
     * @return Received 8-bit data 
     */
    uint8_t transfer(uint8_t data);

    /**
     * @brief Write 16-bit data to the SPI bus and also receive 16-bit data
     * 
     * @param [in] data 16-bit data to send
     * @return Received 16-bit data 
     */
    uint16_t transfer16(uint16_t data);

    /**
     * @brief Write data to the SPI bus and also receive data
     * 
     * @param [in,out] buf Buffer to send and receive
     * @param [in] count The number of bytes to transmit 
     */
    void transfer(void *buf, size_t count);

    /**
     * @brief Write 16-bit data to the SPI bus and also receive data
     *
     * @param [in,out] buf Buffer to send and receive
     * @param [in] count The number of 16-bit data to transmit
     */
    void transfer16(void *buf, size_t count);

    /**
     * @brief Write buffer to the SPI bus (only write transfer)
     *
     * @note  This supports only Tx transfer. There is no Rx received data.
     *        It assumes that this is used for LCD display.
     *
     * @param [in] buf Buffer to send
     * @param [in] count The number of bytes to transmit
     */
    void send(void *buf, size_t count);

    /**
     * @brief Write 16-bit buffer the SPI bus (only write transfer)
     *
     * @note  This supports only Tx transfer. There is no Rx received data.
     *        It assumes that this is used for LCD display.
     *
     * @param [in] buf Buffer to send
     * @param [in] count The number of 16-bit data to transmit
     */
    void send16(void *buf, size_t count);

    /**
     * @brief Select chip select number (only for SPI3)
     *
     * @param [in] cs chip select number
     *             When SPI3_CS0_X is used, set cs to 0 (default).
     *             When SPI3_CS1_X is used, set cs to 1.
     *
     * @note  This is a function supported only for SPI3.
     */
    void selectCS(int cs);

    /**
     * @brief Enable chip select by software (only for SPI3)
     *
     * @note  This is a function supported only for SPI3.
     */
    void enableCS();

    /**
     * @brief Disable chip select by software (only for SPI3)
     *
     * @note  This is a function supported only for SPI3.
     */
    void disableCS();

private:
    int spi_port;                  /**< SPI port number */
    uint8_t ref_count;             /**< Count of SPI references */
    FAR struct spi_dev_s* spi_dev; /**< SPI specific state data */
    uint32_t spi_base_clock;       /**< SPI base clock */
    uint8_t spi_bit_order;         /**< SPI bit order */

    uint8_t spi_transmitting;      /**< Transmitting state */

    SpiInterruptMode interrupt_mode; /**< mode : 0=none, 1=mask, 2=global */

    /**
     * @brief Interrupts to mask
     * 
     * @details 0 indicates bit not used\n
     *          E0 indicates CXD56_IRQ_EXDEVICE_0 + 0\n
     *          E1 indicates CXD56_IRQ_EXDEVICE_0 + 1\n
     *          ...\n
     *          E11 indicates CXD56_IRQ_EXDEVICE_0 + 11\n
     *          bit 15 14 13 12 11  10  09 08 07 06 05 04 03 02 01 00\n
     *              0  0  0  0  E11 E10 E9 E8 E7 E6 E5 E4 E3 E2 E1 E0
     */
    uint16_t interrupt_mask;

    /**
     * @brief Temporary storage to restore state
     * 
     * @details In case some interrupts are disabled before calling usingInterrupt
     */
    uint16_t interrupt_save;

    int spi3_cs1_enable;
};

#if defined(CONFIG_CXD56_SPI4) && defined(CONFIG_SPI_EXCHANGE)
extern SPIClass SPI4;

#ifndef IS_INCLUDED_BY_SPI_CPP
extern SPIClass SPI;
#endif

#else
#error Please enable SPI4 and SPI_EXCHANGE in NuttX
#endif

#if defined(CONFIG_CXD56_SPI5) && defined(CONFIG_SPI_EXCHANGE)
extern SPIClass SPI5;
#endif

#if defined(CONFIG_CXD56_SPI3) && defined(CONFIG_SPI_EXCHANGE)
extern SPIClass SPI3;
#endif

// #endif // __cplusplus

/** @} spi */

#endif // Spi_h
