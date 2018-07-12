/*
  wiring_pulse.c - pulse implement file for the Sparduino SDK
  Copyright (C) 2018 Sony Semiconductor Solutions Corp.
  Copyright (c) 2017 Sony Corporation  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <common/up_arch.h>
#include <Arduino.h>
#include "wiring_private.h"
#include "wiring.h"

/* Measures the length (in microseconds) of a pulse on the pin; state is HIGH
 * or LOW, the type of pulse to measure.  Works on pulses from 2-3 microseconds
 * to 3 minutes in length, but must be called at least a few dozen microseconds
 * before the start of the pulse.
 *
 * This function performs better with short pulses in noInterrupt() context
 */
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout)
{
    // cache the port and bit of the pin in order to speed up the
    // pulse width measuring loop and achieve finer resolution.  calling
    // digitalRead() instead yields much coarser resolution.
    uint8_t _pin = pin_convert(pin);
    if (_pin == PIN_NOT_ASSIGNED) return 0;

    uint32_t regaddr = get_gpio_regaddr(_pin);
    uint32_t regval = getreg32(regaddr);
    uint32_t shift = GPIO_INPUT_SHIFT;

    if (GPIO_OUTPUT_ENABLED(regval))
        shift = GPIO_OUTPUT_SHIFT;

    uint32_t bit = (1 << shift);
    uint32_t state_mask = (state ? bit : 0);

    // convert the timeout from microseconds to a number of times through
    // the initial loop; it takes approximately 32 clock cycles per iteration
    unsigned long maxloops = microsecondsToClockCycles(timeout) / 48;
    unsigned long width = 0;
    // wait for any previous pulse to end
    while ((getreg32(regaddr) & bit) == state_mask) {
        if (--maxloops == 0) {
            //printf("wait for previous pulse end timeout\n");
            return 0;
        }
    }
    // wait for the pulse to start
    while ((getreg32(regaddr) & bit) != state_mask) {
        if (--maxloops == 0) {
            //printf("wait for pulse to start timeout\n");
            return 0;
        }
    }
    // wait for the pulse to stop
    while ((getreg32(regaddr) & bit) == state_mask) {
        if (++width == maxloops) {
            //printf("wait for pulse to stop timeout\n");
            return 0;
        }
    }

    return clockCyclesToMicroseconds(width * 48 + 48);
}

/* Measures the length (in microseconds) of a pulse on the pin; state is HIGH
 * or LOW, the type of pulse to measure.  Works on pulses from 2-3 microseconds
 * to 3 minutes in length, but must be called at least a few dozen microseconds
 * before the start of the pulse.
 *
 * ATTENTION:
 * this function relies on micros() so cannot be used in noInterrupt() context
 */
unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout)
{
    // cache the port and bit of the pin in order to speed up the
    // pulse width measuring loop and achieve finer resolution.  calling
    // digitalRead() instead yields much coarser resolution.
    uint8_t _pin = pin_convert(pin);
    if (_pin == PIN_NOT_ASSIGNED) return 0;

    uint32_t regaddr = get_gpio_regaddr(_pin);
    uint32_t regval = getreg32(regaddr);
    uint32_t shift = GPIO_INPUT_SHIFT;

    if (GPIO_OUTPUT_ENABLED(regval))
        shift = GPIO_OUTPUT_SHIFT;

    uint32_t bit = (1 << shift);
    uint32_t state_mask = (state ? bit : 0);

    uint64_t start_micros = micros();

    // wait for any previous pulse to end
    while ((getreg32(regaddr) & bit) == state_mask) {
        if (micros() - start_micros > timeout) {
            //printf("wait for previous pulse end timeout\n");
            return 0;
        }
    }

    // wait for the pulse to start
    while ((getreg32(regaddr) & bit) != state_mask) {
        if (micros() - start_micros > timeout) {
            //printf("wait for pulse to start timeout\n");
            return 0;
        }
    }

    uint64_t start = micros();
    // wait for the pulse to stop
    while ((getreg32(regaddr) & bit) == state_mask) {
        if (micros() - start_micros > timeout) {
            //printf("wait for pulse to stop timeout\n");
            return 0;
        }
    }
    return micros() - start;
}
