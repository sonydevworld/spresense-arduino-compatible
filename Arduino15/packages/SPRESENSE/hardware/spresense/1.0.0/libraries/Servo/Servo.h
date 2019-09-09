/*
 *  Servo.h - Servo Header file for the Spresense SDK
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

#ifndef Servo_h
#define Servo_h

/**
 * @defgroup servo Servo Library API
 * @brief API for using Servo
 * @{
 */

// #ifdef __cplusplus

#include <stdbool.h>
#include <stdint.h>

// Pin number of unattached pins
#define SERVO_NOT_ATTACHED          (0xFF)

// Default min/max pulse widths (in microseconds) and angles (in degrees).
#define SERVO_MIN_ANGLE             (0)
#define SERVO_MAX_ANGLE             (180)

#define SERVO_MIN_PULSE_WIDTH       (544)     // the shortest pulse sent to a servo
#define SERVO_MAX_PULSE_WIDTH       (2400)    // the longest pulse sent to a servo

#define SERVO_DEFAULT_PULSE_WIDTH   (1500)      // default pulse width when servo is attached
#define SERVO_REFRESH_INTERVAL      (2500)     // minumim time to refresh servos in microseconds
#define SERVO_REFRESH_FREQUENCY     (1000000L / SERVO_REFRESH_INTERVAL)

/** Class for interfacing with RC servomotors. */
class Servo
{
public:
    /**
     * @brief Construct a new Servo instance.
     *
     * The new instance will not be attached to any pin.
     */
    Servo();

    /**
     * @brief Destructor a Servo instance.
     *
     * The instance will be detached if it is attached to any pin.
     */
    ~Servo();

    /**
     * @brief Associate this instance with a servomotor whose input is
     *        connected to pin.
     *
     * If this instance is already attached to a pin, it will be
     * detached before being attached to the new pin. This function
     * doesn't detach any interrupt attached with the pin's timer
     * channel.
     *
     * @param pin Pin connected to the servo pulse wave input. This
     *            pin must be capable of PWM output.
     *
     * @param min_pulse_width Minimum pulse width to write to pin, in
     *                      microseconds.  This will be associated
     *                      with a min_angle degree angle.  Defaults to
     *                      SERVO_DEFAULT_MIN_PW = 544.
     *
     * @param max_pulse_width Maximum pulse width to write to pin, in
     *                      microseconds.  This will be associated
     *                      with a max_angle degree angle. Defaults to
     *                      SERVO_DEFAULT_MAX_PW = 2400.
     *
     * @param min_angle Target angle (in degrees) associated with
     *                 min_pulse_width.  Defaults to
     *                 SERVO_DEFAULT_MIN_ANGLE = 0.
     *
     * @param max_angle Target angle (in degrees) associated with
     *                 max_pulse_width.  Defaults to
     *                 SERVO_DEFAULT_MAX_ANGLE = 180.
     *
     * @sideeffect May set pinMode(pin, PWM).
     *
     * @return true if successful, false when pin doesn't support PWM.
     */
    bool attach(uint8_t pin,
                uint16_t min_pulse_width = SERVO_MIN_PULSE_WIDTH,
                uint16_t max_pulse_width = SERVO_MAX_PULSE_WIDTH,
                uint16_t min_angle = SERVO_MIN_ANGLE,
                uint16_t max_angle = SERVO_MAX_ANGLE);

    /**
     * @brief Stop driving the servo pulse train.
     *
     * If not currently attached to a motor, this function has no effect.
     *
     * @return true if this call did anything, false otherwise.
     */
    bool detach();

    /**
     * @brief Set the servomotor target angle.
     *
     * @param angle Target angle, in degrees.  If the target angle is
     *              outside the range specified at attach() time, it
     *              will be clamped to lie in that range.
     *
     * @see Servo::attach()
     */
    void write(uint16_t angle);

    /**
     * @brief Set the pulse width, in microseconds.
     *
     * @param pulse_width Pulse width to send to the servomotor, in
     *                   microseconds. If outside of the range
     *                   specified at attach() time, it is clamped to
     *                   lie in that range.
     *
     * @see Servo::attach()
     */
    void writeMicroseconds(uint16_t pulse_width);

    /**
     * Get the servomotor's target angle, in degrees.  This will
     * lie inside the range specified at attach() time.
     *
     * @see Servo::attach()
     */
    uint16_t read() const;

    /**
     * @brief Check if this instance is attached to a servo.
     * @return true if this instance is attached to a servo, false otherwise.
     * @see Servo::attachedPin()
     */
    bool attached() const { return this->pin_ != SERVO_NOT_ATTACHED; }

    /**
     * @brief Get the pin this instance is attached to.
     * @return Pin number if currently attached to a pin, NOT_ATTACHED
     *         otherwise.
     * @see Servo::attach()
     */
    uint8_t attachedPin() const { return this->pin_; }

private:
    uint8_t pin_;
    uint16_t min_pw_;
    uint16_t max_pw_;
    uint16_t min_angle_;
    uint16_t max_angle_;
    uint16_t angle_;
};

// #endif // __cplusplus

/** @} servo */

#endif //Servo_h
