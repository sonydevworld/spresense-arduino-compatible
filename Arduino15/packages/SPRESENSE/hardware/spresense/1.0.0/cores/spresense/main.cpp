/*
  main.cpp - Main loop for Arduino sketches
  Copyright (C) 2018 Sony Semiconductor Solutions Corp.
  Copyright (c) 2005-2013 Arduino Team.  All right reserved.

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

#include <nuttx/config.h>
#include <sdk/config.h>
#include <stdint.h>
extern "C" { // boardctl.h forget to declare extern "C"...
#include <sys/boardctl.h>
}
#include <Arduino.h>
#include "multi_print.h"

extern "C" {

// Declared weak in Arduino.h to allow user redefinitions.
int atexit(void (*func)()) __attribute__((weak));
int atexit(void (* /*func*/ )()) { return 0; }

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant() __attribute__((weak));
void initVariant() { }

void setupUSB() __attribute__((weak));
void setupUSB() { }

void serialEventRun(void);
void serialEvent(void);

int spresense_main(int argc, char *argv[])
{
    int r = boardctl(BOARDIOC_INIT, 0);
    if (r) printf("WARNING: Something wrong during board initialization\n");

    initVariant();

#if defined(USBCON)
    USBDevice.attach();
#endif

    /* Initialize log output for MultiCore */
    init_multi_print();

    setup();

    for (;;) {
        loop();
        if ((NULL != (void *)serialEvent) && (NULL != (void *)serialEventRun))
            serialEventRun();
    }

    return 0;
}

} // extern "C"
