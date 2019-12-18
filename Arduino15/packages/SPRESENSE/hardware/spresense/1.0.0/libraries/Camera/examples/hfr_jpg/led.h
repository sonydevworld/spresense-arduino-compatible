/*
 *  led.h - Handling LED oeration
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

#ifndef __CAMERA_EXAMPLES_HFR_JPG_LED_H__
#define __CAMERA_EXAMPLES_HFR_JPG_LED_H__

/**
 * @file led.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Handling LED operation
 */

/**
 * @brief Initialize all LEDs = ON.
 * 
 * @param [in] max_count MAX of frame counter
 */

void led_init(int max_count);

/**
 * @brief Control LEDs for notification of remaining time.
 * 
 * @param [in] count frame counter
 */

void led_update(int count);

#endif /* __CAMERA_EXAMPLES_HFR_JPG_LED_H__ */

