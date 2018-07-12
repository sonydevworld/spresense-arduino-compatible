/*
 *  Servo.cpp - Servo Implementation file for the Spresense SDK
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

#include <stdio.h>
#include <Arduino.h>
#include "utility.h"
#include "wiring_private.h"
#include "Servo.h"

#undef LOG_PREFIX
#define LOG_PREFIX  "SERV"

// Unit conversions
#define ANGLE_TO_US(a)    ((uint16_t)(map((a), min_angle_, max_angle_, min_pw_, max_pw_)))
#define US_TO_ANGLE(us)   ((uint16_t)(map((us), min_pw_, max_pw_, min_angle_, max_angle_)))

Servo::Servo()
  : pin_(SERVO_NOT_ATTACHED),
    min_pw_(SERVO_MIN_PULSE_WIDTH),
    max_pw_(SERVO_MAX_PULSE_WIDTH),
    min_angle_(SERVO_MIN_ANGLE),
    max_angle_(SERVO_MAX_ANGLE),
    angle_(SERVO_MIN_ANGLE)
{
}

Servo::~Servo()
{
    detach();
}

bool Servo::attach(uint8_t pin, uint16_t min_pw, uint16_t max_pw, uint16_t min_angle, uint16_t max_angle)
{
    if (pin_convert(pin) == PIN_NOT_ASSIGNED)
        return false;

    (void) detach();
    pin_ = pin;
    min_pw_ = min_pw;
    max_pw_ = max_pw;
    min_angle_ = min_angle;
    max_angle_ = max_angle;

    return true;
}

bool Servo::detach()
{
    if (!attached())
        return false;

    analog_stop(pin_);
    return true;
}

void Servo::write(uint16_t angle)
{
    angle = constrain(angle, min_angle_, max_angle_);
    writeMicroseconds(ANGLE_TO_US(angle));
}

void Servo::writeMicroseconds(uint16_t pulse_width)
{
    if (!attached())
        return;

    pulse_width = constrain(pulse_width, min_pw_, max_pw_);
    angle_ = US_TO_ANGLE(pulse_width);
    analog_write(pin_, pulse_width, SERVO_REFRESH_FREQUENCY);
}

uint16_t Servo::read() const
{
    return angle_;
}
