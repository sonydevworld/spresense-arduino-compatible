/*
 *  SPI.cpp - Spresense Arduino SPI library 
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

/**
 * @file SPI.cpp
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino SPI library 
 * 
 * @details It is a library for communicating with SPI devices, with the 
 *          Spresense as the master device. 
 */

#include <sdk/config.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <nuttx/compiler.h>
#include <nuttx/arch.h>
#include <arch/cxd56xx/irq.h>
#include <cxd56_spi.h>
#include <chip/hardware/cxd5602_memorymap.h>
#include <chip/cxd56_spi.h>
#include <cxd56_clock.h>
#include "wiring_private.h"

#define IS_INCLUDED_BY_SPI_CPP  /**< Included by SPI CPP */ 
#include "SPI.h"

#define SPIDEV_SPRESENSE  (SPIDEV_USER(0))  /**< Identifies the device to select */ 
#define SPIDEV_PORT_3     (3)               /**< SPI3 */
#define SPIDEV_PORT_4     (4)               /**< SPI4 */ 
#define SPIDEV_PORT_5     (5)               /**< SPI5 */ 

#define SPI_INT_BASE      (CXD56_IRQ_EXDEVICE_0)       /**< SPI base interrupt number */
#define SPI_INT_MAX       (CXD56_IRQ_EXDEVICE_0 + 12)  /**< SPI max interrupt number */

#ifdef CONFIG_CXD56_SPI4
SPIClass SPI4(SPIDEV_PORT_4);

// SPI object will point to SPI4 by default
extern SPIClass SPI __attribute__((alias("SPI4")));
#endif

#ifdef CONFIG_CXD56_SPI5
SPIClass SPI5(SPIDEV_PORT_5);
#endif

#ifdef CONFIG_CXD56_SPI3
SPIClass SPI3(SPIDEV_PORT_3);
#endif

/**
 * @brief Reverse bits.
 */
static inline uint8_t reverse_bits(uint8_t data) inline_function;
static inline uint8_t reverse_bits(uint8_t data)
{
    return (((data & 0x01) << 7) | ((data & 0x02) << 5) | ((data & 0x04) << 3) | ((data & 0x08) << 1) |
            ((data & 0x10) >> 1) | ((data & 0x20) >> 3) | ((data & 0x40) >> 5) | ((data & 0x80) >> 7));
}

#define lsb2msb(d)  reverse_bits(d) /**< Reverse bits (LSB to MSB). */
#define msb2lsb(d)  reverse_bits(d) /**< Reverse bits (MSB to LSB). */

SPIClass::SPIClass(int port)
:spi_port(port),
 ref_count(0),
 spi_dev(0),
 spi_base_clock(0),
 spi_bit_order(MSBFIRST),
 spi_transmitting(0),
 interrupt_mode(SPI_INT_MODE_NONE),
 interrupt_mask(0),
 interrupt_save(0)
{
}

void SPIClass::begin(void)
{
    if (ref_count == 0) {
        if (!spi_dev) {
            spi_dev = cxd56_spibus_initialize(spi_port);
            if (!spi_dev) {
                printf("Failed to initialize SPI bus on port %d!\n", spi_port);
                return;
            }
            if (spi_port == SPIDEV_PORT_3) {
                /* Control CS by hardware */
                cxd56_spi_clock_gate_disable(3);
                *(volatile uint32_t*)CXD56_SPI3_CSMODE = 0;
                cxd56_spi_clock_gate_enable(3);
                /* Disable SPI3_CS1_X by default */
                spi3_cs1_enable = 0;
            }

            spi_base_clock = cxd56_get_spi_baseclock(spi_port);
            spi_bit_order = MSBFIRST;
        }
    }

    ++ref_count;
}

void SPIClass::end(void)
{
    if (ref_count > 0)
        --ref_count;

    if (ref_count == 0) {
        (void) SPI_LOCK(spi_dev, false);
        interrupt_mode = SPI_INT_MODE_NONE;
    }
}

void SPIClass::beginTransaction(SPISettings settings)
{
    if (ref_count == 0) return;

    if (interrupt_mode) {
        noInterrupts();
        if (interrupt_mode == SPI_INT_MODE_MASK) {
            interrupt_save = irq_save(interrupt_mask);
            interrupts();
        }
    }

    if (SPI_LOCK(spi_dev, true)) {
        printf("ERROR: Failed to lock spi bus (errno = %d)\n", errno);
        return;
    }

    SPI_SETMODE(spi_dev, static_cast<enum spi_mode_e>(settings.data_mode_));
    SPI_SETBITS(spi_dev, 8);
    SPI_SETFREQUENCY(spi_dev, settings.clock_);
    spi_bit_order = (settings.bit_order_ == LSBFIRST ? LSBFIRST : MSBFIRST);

    spi_transmitting = true;

    //printf("SPI transaction: mode [%u], freq [%u], bit order [%u]\n", settings.data_mode_, freq, settings.bit_order_);
}

void SPIClass::endTransaction(void)
{
    if (ref_count == 0) return;

    if (SPI_LOCK(spi_dev, false)) {
        printf("ERROR: Failed to unlock spi bus (errno = %d)\n", errno);
        return;
    }

    if (interrupt_mode) {
        noInterrupts();
        if (interrupt_mode == SPI_INT_MODE_MASK)
            irq_restore(interrupt_save);
        interrupts();
    }
    spi_transmitting = false;
}

void SPIClass::setBitOrder(uint8_t bitOrder)
{
    if (ref_count == 0) return;

    SPI_SETBITS(spi_dev, 8);
    spi_bit_order = (bitOrder == LSBFIRST ? LSBFIRST : MSBFIRST);
}

void SPIClass::setDataMode(uint8_t dataMode)
{
    if (ref_count == 0) return;

    SPI_SETMODE(spi_dev, static_cast<enum spi_mode_e>(dataMode));
}

void SPIClass::setClockDivider(uint8_t clockDiv)
{
    if (ref_count == 0) return;

    (void) SPI_SETFREQUENCY(spi_dev, spi_base_clock / clockDiv);
}

void SPIClass::usingInterrupt(uint8_t interruptNumber)
{
    if (up_interrupt_context() || spi_transmitting) {
        printf("WARNING: usingInterrupt should NOT be called from ISR context or inside a transaction\n");
        return;
    }

    uint8_t mask = 0;
    noInterrupts();
    if (interruptNumber >= SPI_INT_BASE && interruptNumber < SPI_INT_MAX)
        mask = interruptNumber - SPI_INT_BASE;
    else
        interrupt_mode = SPI_INT_MODE_GLOBAL;

    if (interrupt_mode == SPI_INT_MODE_NONE)
        interrupt_mode = SPI_INT_MODE_MASK;
    interrupt_mask |= (1 << mask);
    interrupts();
}

void SPIClass::notUsingInterrupt(uint8_t interruptNumber)
{
    if (up_interrupt_context() || spi_transmitting) {
        printf("WARNING: notUsingInterrupt should NOT be called from ISR context or inside a transaction\n");
        return;
    }

    if (interrupt_mode == SPI_INT_MODE_GLOBAL)
        return;

    if (interruptNumber >= SPI_INT_BASE && interruptNumber < SPI_INT_MAX) {
        noInterrupts();
        uint8_t mask = interruptNumber - SPI_INT_BASE;
        interrupt_mask &= ~(1 << mask);
        if (interrupt_mask == 0)
            interrupt_mode = SPI_INT_MODE_NONE;
        interrupts();
    }
}

uint8_t SPIClass::transfer(uint8_t data)
{
    if (ref_count == 0) return 0;

    uint8_t received = 0;
    if (spi_bit_order == LSBFIRST) data = lsb2msb(data);
    SPI_SETBITS(spi_dev, 8);
    SPI_EXCHANGE(spi_dev, (void*)(&data), (void*)(&received), 1);
    if (spi_bit_order == LSBFIRST) received = msb2lsb(received);

    return received;
}

uint16_t SPIClass::transfer16(uint16_t data)
{
    if (ref_count == 0) return 0;

    union {
        uint16_t val;
        struct {
            uint8_t hi;
            uint8_t lo;
        };
    } in, out;

    in.val = data;
    if (spi_bit_order == LSBFIRST) {
        in.hi = lsb2msb(in.hi);
        in.lo = lsb2msb(in.lo);
    }

    SPI_SETBITS(spi_dev, 16);
    SPI_EXCHANGE(spi_dev, (void*)(&in.hi), (void*)(&out.hi), 1);

    if (spi_bit_order == LSBFIRST) {
        out.hi = msb2lsb(out.hi);
        out.lo = msb2lsb(out.lo);
    }

    return out.val;
}

void SPIClass::transfer(void *buf, size_t count)
{
    bool do_free = false;
    uint8_t *recv = NULL;
    if (ref_count == 0 || !buf || count == 0)
        return;

    if (count > 256) {
        recv = (uint8_t*)malloc(count);
        do_free = true;
        if (!recv)
            return;
    }
    else {
        recv = (uint8_t*)__builtin_alloca(count);
    }

    if (spi_bit_order == LSBFIRST) {
        uint8_t* p = (uint8_t*)buf;
        for (size_t i = 0; i < count; ++i, ++p) {
            *p = lsb2msb(*p);
        }
    }

    SPI_SETBITS(spi_dev, 8);
    SPI_EXCHANGE(spi_dev, buf, recv, count);

    if (spi_bit_order == LSBFIRST) {
        uint8_t* p = (uint8_t*)recv;
        for (size_t i = 0; i < count; ++i, ++p) {
            *p = msb2lsb(*p);
        }
    }

    memcpy(buf, recv, count);
    if (do_free)
        free(recv);
}

void SPIClass::transfer16(void *buf, size_t count)
{
    bool do_free = false;
    uint16_t *recv = NULL;
    if (ref_count == 0 || !buf || count == 0)
        return;

    if ((count * sizeof(uint16_t)) > 256) {
        recv = (uint16_t*)malloc(count * sizeof(uint16_t));
        do_free = true;
        if (!recv)
            return;
    }
    else {
        recv = (uint16_t*)__builtin_alloca(count * sizeof(uint16_t));
    }

    SPI_SETBITS(spi_dev, 16);
    SPI_EXCHANGE(spi_dev, buf, recv, count);

    memcpy(buf, recv, count * sizeof(uint16_t));
    if (do_free)
        free(recv);
}

void SPIClass::send(void *buf, size_t count)
{
    if (ref_count == 0 || !buf || count == 0)
        return;

    SPI_SETBITS(spi_dev, 8);
    SPI_EXCHANGE(spi_dev, buf, NULL, count);
}

void SPIClass::send16(void *buf, size_t count)
{
    if (ref_count == 0 || !buf || count == 0)
        return;

    SPI_SETBITS(spi_dev, 16);
    SPI_EXCHANGE(spi_dev, buf, NULL, count);
}

void SPIClass::selectCS(int cs)
{
    if ((cs != 0) && (cs != 1)) {
        return;
    }
    if (spi_port == SPIDEV_PORT_3) {
        if ((cs == 1) && (spi3_cs1_enable == 0)) {
            /* Enable SPI3_CS1_X */
            CXD56_PIN_CONFIGS(PINCONFS_SPI3_CS1_X);
        }
        cxd56_spi_clock_gate_disable(3);
        *(volatile uint32_t*)CXD56_SPI3_SLAVETYPE = cs;
        cxd56_spi_clock_gate_enable(3);
    }
}

void SPIClass::enableCS()
{
    if (spi_port == SPIDEV_PORT_3) {
        /* Control CS by software */
        cxd56_spi_clock_gate_disable(3);
        *(volatile uint32_t*)CXD56_SPI3_CSMODE = 1;
        *(volatile uint32_t*)CXD56_SPI3_CS = 0;
        cxd56_spi_clock_gate_enable(3);
    }
}

void SPIClass::disableCS()
{
    if (spi_port == SPIDEV_PORT_3) {
        /* Control CS by software */
        cxd56_spi_clock_gate_disable(3);
        *(volatile uint32_t*)CXD56_SPI3_CSMODE = 1;
        *(volatile uint32_t*)CXD56_SPI3_CS = 1;
        cxd56_spi_clock_gate_enable(3);
    }
}
