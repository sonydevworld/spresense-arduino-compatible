/*
 *  SubShell.ino - MP Example for NuttShell on SubCore
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

#ifndef SUBCORE
#error "Core selection is wrong!!"
#endif

#include <MP.h>
#include <sched.h>
#include "nshlib.h"

#define STACKSIZE 8192
#define PRIORITY  100

static int nsh_main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  return nsh_consolemain(0, NULL);
}

void setup() {
  MP.begin();
  MP.EnableConsole();
  task_create("nsh", PRIORITY, STACKSIZE, nsh_main, NULL);
}

void loop() {
  exit(0);
}
