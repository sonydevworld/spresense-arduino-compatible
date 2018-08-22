/*
 *  Servo.ino - Servo sample application
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
 * @file Servo.ino
 * @author Sony Semiconductor Solutions Corporation
 * @brief %Servo sample application.
 */

#include <Servo.h>

static Servo s_servo; /**< Servo object */

/**
 * @brief Initialize servo motor.
 */
void setup() {
  /* put your setup code here, to run once: */

  /* Attached to a servo motor whose input is connected to PIN_D09 
     Note: The pin selected must support PWM. */
  s_servo.attach(PIN_D09);

  /* Set the servo motor angle to 90 degrees */
  s_servo.write(90);

  /* Wait 5000ms */
  delay(5000);
}

/**
 * @brief Change servo motor angle.
 */
void loop() {
  /* put your main code here, to run repeatedly: */

  /* Set the servo motor angle to 0 degrees */
  s_servo.write(0);

  /* Wait 1000ms */
  delay(1000);

  /* Set the servo motor angle to 180 degrees */
  s_servo.write(180);

  /* Wait 1000ms */
  delay(1000);
}
