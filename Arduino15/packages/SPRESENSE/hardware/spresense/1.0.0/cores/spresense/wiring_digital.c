/*
  wiring_digital.c - digital I/O for the Sparduino SDK
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

#include <stdbool.h>
#include <stdio.h>
#include <sdk/config.h>
#include <arch/board/board.h>
#include <arch/chip/pin.h>
#include <chip/cxd5602_memorymap.h>
#include <chip/cxd5602_topreg.h>
#include <common/up_arch.h>
#include <Arduino.h>
#include "utility.h"
#include "wiring_private.h"

uint32_t get_gpio_regaddr(uint32_t pin)
{
    uint32_t base;

    base = (pin < PIN_IS_CLK) ? 1 : 7;

    return CXD56_TOPREG_GP_I2C4_BCK + ((pin - base) * 4);
}

uint8_t pin_convert(uint8_t pin)
{
    static const struct {
        uint8_t pin;
        uint8_t mapped;
    } pin_maps[] = {
        { PIN_D00, PIN_UART2_RXD     },
        { PIN_D01, PIN_UART2_TXD     },
        { PIN_D02, PIN_HIF_IRQ_OUT   },
        { PIN_D03, PIN_PWM3          },
        { PIN_D04, PIN_SPI2_MOSI     },
        { PIN_D05, PIN_PWM1          },
        { PIN_D06, PIN_PWM0          },
        { PIN_D07, PIN_SPI3_CS1_X    },
        { PIN_D08, PIN_SPI2_MISO     },
        { PIN_D09, PIN_PWM2          },
        { PIN_D10, PIN_SPI4_CS_X     },
        { PIN_D11, PIN_SPI4_MOSI     },
        { PIN_D12, PIN_SPI4_MISO     },
        { PIN_D13, PIN_SPI4_SCK      },
        { PIN_D14, PIN_I2C0_BDT      },
        { PIN_D15, PIN_I2C0_BCK      },
        { PIN_D16, PIN_EMMC_DATA0    },
        { PIN_D17, PIN_EMMC_DATA1    },
        { PIN_D18, PIN_I2S0_DATA_OUT },
        { PIN_D19, PIN_I2S0_DATA_IN  },
        { PIN_D20, PIN_EMMC_DATA2    },
        { PIN_D21, PIN_EMMC_DATA3    },
        { PIN_D22, PIN_SEN_IRQ_IN    },
        { PIN_D23, PIN_EMMC_CLK      },
        { PIN_D24, PIN_EMMC_CMD      },
        { PIN_D25, PIN_I2S0_LRCK     },
        { PIN_D26, PIN_I2S0_BCK      },
        { PIN_D27, PIN_UART2_CTS     },
        { PIN_D28, PIN_UART2_RTS     },
        { PIN_LED0, PIN_I2S1_BCK     },
        { PIN_LED1, PIN_I2S1_LRCK    },
        { PIN_LED2, PIN_I2S1_DATA_IN },
        { PIN_LED3, PIN_I2S1_DATA_OUT},
    };

    arrayForEach(pin_maps, i) {
        if (pin_maps[i].pin == pin)
            return pin_maps[i].mapped;
    }

    printf("ERROR: Invalid pin number [%u]\n", pin);
    if ((pin & PINTYPE_MASK) == PINTYPE_ANALOG) {
        printf("\tspresense dose not support using analog pin as digital.\n");
    }
    return PIN_NOT_ASSIGNED;
}

void fast_digital_write(uint32_t reg_addr, uint8_t value)
{
    uint32_t reg_val = getreg32(reg_addr);
    bitWrite(reg_val, GPIO_OUTPUT_SHIFT, value);
    putreg32(reg_val, reg_addr);
}

bool fast_digital_read(uint32_t reg_addr)
{
    uint32_t shift = 0;
    uint32_t reg_val = getreg32(reg_addr);
    if (GPIO_OUTPUT_ENABLED(reg_val))
        shift = GPIO_OUTPUT_SHIFT;
    else
        shift = GPIO_INPUT_SHIFT;

    return ((reg_val & (1 << shift)) != 0);
}

void pinMode(uint8_t pin, uint8_t mode)
{
    uint8_t _pin = pin_convert(pin);
    if (_pin == PIN_NOT_ASSIGNED)
        return;

    bool input;
    bool highdrive = true; // always use high drive current
    int  pull;

    switch (mode) {
    case INPUT:
        input = true;
        pull = PIN_FLOAT;
        break;
    case OUTPUT:
        input = false;
        pull = PIN_FLOAT;
        break;
    case INPUT_PULLUP:
        input = true;
        pull = PIN_PULLUP;
        break;
    case INPUT_PULLDOWN:
        input = true;
        pull = PIN_PULLDOWN;
        break;
    default:
        printf("ERROR: unknown pin mode [%d]\n", mode);
        return;
    }

    /* disable output, it will be enabled on digitalWrite call */
    board_gpio_write(_pin, -1);
    board_gpio_config(_pin, 0, input, highdrive, pull);
}

void digital_write(uint8_t pin, uint8_t value, uint8_t stop_pwm)
{
    uint8_t _pin = pin_convert(pin);
    if (_pin == PIN_NOT_ASSIGNED)
        return;

    value = (value == LOW ? LOW : HIGH);
    if (stop_pwm) analog_stop(pin);
    // board_gpio_write will enable output
    board_gpio_write(_pin, value);
}

void digitalWrite(uint8_t pin, uint8_t value)
{
    digital_write(pin, value, true);
}

int digitalRead(uint8_t pin)
{
    uint8_t _pin = pin_convert(pin);
    if (_pin == PIN_NOT_ASSIGNED)
        return LOW;

    analog_stop(pin);
    return board_gpio_read(_pin);
}
