/*
 *  LTE.h - LTE include file for Spresense Arduino
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

/**
 * @file LTE.h
 *
 * @author Sony Semiconductor Solutions Corporation
 *
 * @brief LTE Library for Spresense Arduino.
 *
 * @details [en] This file simplifies the use of LTE library classes.
 *               When using the LTE library class, include this file first.
 *
 * @details [ja] LTEライブラリクラスの利用を簡略化するためのファイルです。
 *               LTEライブラリクラスを使用する場合、本ファイルをはじめにインクルードしてください。
 */

#ifndef __SPRESENSE_LTE_H__
#define __SPRESENSE_LTE_H__

#ifdef SUBCORE
#error "LTE library is NOT supported by SubCore."
#endif

/**
 * @defgroup lte LTE Library API
 *
 * @brief API for using LTE
 * @{
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "LTEAccessProvider.h"
#include "LTEModemVerification.h"
#include "LTEScanNetworks.h"
#include "LTEClient.h"
#include "LTETLSClient.h"
#include "LTEUDP.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/**
 * @defgroup LTE_CLASS_NAME LTE class name definitions
 *
 * @brief LTE class name definition.
 * @{
 */
#define LTE        LTEAccessProvider
#define LTEModem   LTEModemVerification
#define LTEScanner LTEScanNetworks

/** @} */

/** @} */

#endif // __SPRESENSE_LTE_H__
