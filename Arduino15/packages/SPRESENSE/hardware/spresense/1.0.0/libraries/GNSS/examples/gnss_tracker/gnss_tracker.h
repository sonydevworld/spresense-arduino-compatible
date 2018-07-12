/*
 *  gnss_tracker.h - Debug log setup
 *  Copyright 2017 Sony Semiconductor Solutions Corporation
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

#ifndef _GNSS_TRACKER_H
#define _GNSS_TRACKER_H

/**
 * @file gnss_tracker.h
 * @author Sony Corporation
 * @brief Debug log setup
 */

/**
 * @enum AppPrintLevel
 * @brief Set the debug log output level
 */
enum AppPrintLevel {
    AppPrintNone = 0,  /**< Log output level none */
    AppPrintError,     /**< Log output level error */
    AppPrintWarning,   /**< Log output level warning */
    AppPrintInfo,      /**< Log output level info */
};

extern AppPrintLevel AppDebugPrintLevel;  /**< %Print level */

//#define APP_DEBUG

#define APP_PRINT(c) Serial.print(c)  /**< %Print log */
#ifdef APP_DEBUG /* switch debug message on/off */
# define APP_PRINT_E(c) if(AppPrintError   <= AppDebugPrintLevel){ Serial.print(c); }  /**< %Print error */
# define APP_PRINT_W(c) if(AppPrintWarning <= AppDebugPrintLevel){ Serial.print(c); }  /**< %Print warning */
# define APP_PRINT_I(c) if(AppPrintInfo    <= AppDebugPrintLevel){ Serial.print(c); }  /**< %Print info */
#else
# define APP_PRINT_E(c)  /**< %Print error */
# define APP_PRINT_W(c)  /**< %Print warning */
# define APP_PRINT_I(c)  /**< %Print info */
#endif /* APP_DEBUG */

#endif

