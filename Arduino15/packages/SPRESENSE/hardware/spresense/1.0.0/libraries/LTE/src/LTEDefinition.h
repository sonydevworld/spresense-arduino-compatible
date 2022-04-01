/*
 *  LTEDefinition.h - LTEDefinition include file for Spresense Arduino
 *  Copyright 2019, 2021 Sony Semiconductor Solutions Corporation
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
 * @file LTEDefinition.h
 *
 * @author Sony Semiconductor Solutions Corporation
 *
 * @brief Definition used in LTE library
 */

#ifndef __SPRESENSE_LTEDEFINITION_H__
#define __SPRESENSE_LTEDEFINITION_H__

/**
 * @defgroup lte LTE Library API
 *
 * @brief API for using LTE
 * @{
 */

#include <string.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef BRD_DEBUG
#define LTEDBG(format, ...) ::printf("DEBUG: " format, ##__VA_ARGS__)
#else
#define LTEDBG(format, ...)
#endif
#define LTEERR(format, ...) ::printf("ERROR: " format, ##__VA_ARGS__)

/**
 * @defgroup APN_LENGTH APN parameter max length definitions
 *
 * @brief APN length definitions.
 * @{
 */

#define LTE_NET_APN_MAXLEN      (101) /** [en] Access Point Name max length */
#define LTE_NET_USER_MAXLEN      (64) /** [en] User Name max length */
#define LTE_NET_PASSWORD_MAXLEN  (32) /** [en] Password max length */

/** @} */

/**
 * @enum LTEModemStatus
 *
 * @brief [en] Status code of the modem. <BR>
 *        [ja] モデムのステータスコード
 */
enum LTEModemStatus {
  LTE_ERROR = 0,  /**< [en] Error */
  LTE_IDLE,       /**< [en] Powered on */
  LTE_CONNECTING, /**< [en] Connecting network */
  LTE_SEARCHING,  /**< [en] Ready to connect to the APN */
  LTE_READY,      /**< [en] APN conneced */
  LTE_OFF         /**< [en] Powered off */
};

/**
 * @enum LTENetworkIPType
 *
 * @brief [en] Internet protocol type. <BR>
 *        [ja] インターネット・プロトコル形式
 */
enum LTENetworkIPType {
  LTE_NET_IPTYPE_V4 = 0, /**< [en] IPv4 */
  LTE_NET_IPTYPE_V6,     /**< [en] IPv6 */
  LTE_NET_IPTYPE_V4V6    /**< [en] IPv4/v6 */
};

/**
 * @enum LTENetworkAuthType
 *
 * @brief [en] Authentication type. <BR>
 *        [ja] 認証形式
 */

enum LTENetworkAuthType {
  LTE_NET_AUTHTYPE_NONE = 0, /**< [en] No authentication */
  LTE_NET_AUTHTYPE_PAP,      /**< [en] PAP */
  LTE_NET_AUTHTYPE_CHAP      /**< [en] CHAP */
};

/** @} */

/**
 * @enum LTENetworkRatType
 *
 * @brief  RAT(Radio Access Technology)
 */
enum LTENetworkRatType {
  LTE_NET_RAT_UNKNOWN = -1, /**< [en] RAT(Radio Access Technology): Unknown */
  LTE_NET_RAT_CATM    = 2,  /**< [en] RAT(Radio Access Technology): LTE-M (LTE Cat-M1) */
  LTE_NET_RAT_NBIOT   = 3   /**< [en] RAT(Radio Access Technology): NB-IoT */
};

#endif // __SPRESENSE_LTEDEFINITION_H__
