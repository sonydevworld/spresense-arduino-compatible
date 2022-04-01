/*
 *  LTECore.h - LTECore include file for Spresense Arduino
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
 * @file LTECore.h
 *
 * @author Sony Semiconductor Solutions Corporation
 *
 * @brief Core class of LTE library. This is internal class.
 */

#ifndef __SPRESENSE_LTECORE_H__
#define __SPRESENSE_LTECORE_H__

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

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

#include <LTEDefinition.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define LTE_NET_PINCODE_MAXLEN    (9)

/****************************************************************************
 * class declaration
 ****************************************************************************/

/**
 * @class LTECore
 *
 * @brief [en] Core class of LTE library. This is internal class. <BR>
 *        [ja] LTEライブラリのCoreクラス。内部利用Class。
 *
 * @attention [en] LTECore class is for internal use only. Do not get an instance from the sketch.
 *
 * @attention [ja] このクラスは内部利用に限定されます。スケッチからインスタンスを取得しないでください。
 */

class LTECore {

public:

  ~LTECore();

  static LTECore getInstance();
  void setStatus(LTEModemStatus status)
  {
    _networkStatus = status;
  }

  inline LTEModemStatus getStatus()
  {
    return _networkStatus;
  }

  inline void setSessionID(uint8_t id)
  {
    _sessionID = id;
  }

  LTEModemStatus begin(bool restart);
  void shutdown();
  LTEModemStatus startSearchNetwork(char* pinCode, bool synchronous);
  LTEModemStatus connectNetwork(const char *apn, const char *userName, const char *password, LTENetworkAuthType authType, LTENetworkIPType ipType, bool synchronous, bool cancelable = true);
  LTEModemStatus disconnectNetwork();
  void signalModemReset();
  void printErrorInfo();
  void recovery();

private:
  static LTECore* _instance;
  LTEModemStatus  _networkStatus;
  uint8_t         _sessionID;
  char            _modemPinCode[LTE_NET_PINCODE_MAXLEN];

  struct {
    pthread_cond_t  cond;
    pthread_mutex_t mutex;
  }_resetCondition;

  struct {
    char               name[LTE_NET_APN_MAXLEN];
    char               userName[LTE_NET_USER_MAXLEN];
    char               password[LTE_NET_PASSWORD_MAXLEN];
    LTENetworkAuthType authType;
    LTENetworkIPType   ipType;
  }_apn;

  LTECore();

};

/* LTECore singleton instance.
 * Do not reference this instance directly from the sketch.
 */
extern LTECore theLTECore;

/** @} */

#endif // __SPRESENSE_LTECORE_H__
