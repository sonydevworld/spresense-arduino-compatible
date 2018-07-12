/*
  wiring_private.h - digital I/O private file for the Sparduino SDK
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

#ifndef Wiring_private_h
#define Wiring_private_h

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

/* GPIO register Definitions */
#define GPIO_OUTPUT_EN_SHIFT    (16)
#define GPIO_OUTPUT_EN_MASK     (1u << GPIO_OUTPUT_EN_SHIFT)
#define GPIO_OUTPUT_ENABLE      (0u << GPIO_OUTPUT_EN_SHIFT)
#define GPIO_OUTPUT_DISABLE     (1u << GPIO_OUTPUT_EN_SHIFT)
#define GPIO_OUTPUT_ENABLED(v)  (((v) & GPIO_OUTPUT_EN_MASK) == GPIO_OUTPUT_ENABLE)
#define GPIO_OUTPUT_SHIFT       (8)
#define GPIO_OUTPUT_MASK        (1u << GPIO_OUTPUT_SHIFT)
#define GPIO_OUTPUT_HIGH        (1u << GPIO_OUTPUT_SHIFT)
#define GPIO_OUTPUT_LOW         (0u << GPIO_OUTPUT_SHIFT)
#define GPIO_INPUT_SHIFT        (0)
#define GPIO_INPUT_MASK         (1u << GPIO_INPUT_SHIFT)

// convert physical pin number to CXD5602 internal pin number
uint8_t pin_convert(uint8_t pin);

// get gpio register address via CXD5602 internal pin number
uint32_t get_gpio_regaddr(uint32_t pin);

// start analog output on physical pin number, with pulse width and frequency
void analog_write(uint8_t pin, uint32_t pulse_width, uint32_t freq);

// stop analog output on physical pin number
void analog_stop(uint8_t pin);

// digital output on physical pin number
// stop_pwm: 0 - do not stop analog output on the pin
//           1 - stop analog output on the pin
void digital_write(uint8_t pin, uint8_t value, uint8_t stop_pwm);

void fast_digital_write(uint32_t reg_addr, uint8_t value);
bool fast_digital_read(uint32_t reg_addr);

// save current irq status, and disable irqs with mask
uint16_t irq_save(uint16_t mask);

// restore irq with flags
void irq_restore(uint16_t flags);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // Wiring_private_h
