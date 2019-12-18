/*
 *  led.cpp - Turn off 4 leds on SPRESENSE board as time goes by.
 *  Copyright 2019 Sony Semiconductor Solutions Corporation
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

#include <Arduino.h>
#include "led.h"

static int g_max_count;

/**
 * Initialize all LEDs = ON.
 */

void led_init(int max_count)
{
  g_max_count = max_count;
  ledOn(LED0);
  ledOn(LED1);
  ledOn(LED2);
  ledOn(LED3);

  return;
}

/**
 * Control LEDs for notification of remaining time.
 */

void led_update(int count)
{
  if (count == g_max_count/4)
    {
      ledOff(LED3);
    }
  else if (count == g_max_count/2)
    {
      ledOff(LED2);
    }
  else if (count == g_max_count * 3/4)
    {
      ledOff(LED1);
    }
  else if (count == g_max_count)
    {
      ledOff(LED0);
    }
  else
    {
      /* Not update LED */
    }

  return;
}
