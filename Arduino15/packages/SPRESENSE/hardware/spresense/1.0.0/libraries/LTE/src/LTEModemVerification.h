/*
 *  LTEModemVerification.h - LTEModemVerification include file for Spresense Arduino
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
 * @file LTEModemVerification.h
 *
 * @author Sony Semiconductor Solutions Corporation
 *
 * @brief Modem information management class for Spresense Arduino.
 *
 * @attention [en] Use LTE.h instead of including this header file directly.
 *
 * @attention [ja] このヘッダファイルを直接インクルードせずに、 LTE.h を使用してください。
 *
 * @details [en] By using this class, you can use the follow features on SPRESSENSE.
 *           - Feature to get IMEI(International Mobile Equipment Identity) of the modem.
 *           - Feature to get firmware verion of the modem.
 *
 * @details [ja] このクラスを使用することで、以下の機能をSPRESENSE上で利用することが出来ます。
 *           - モデムのIMEI(International Mobile Equipment Identity)を取得する機能。
 *           - モデムのファームウェアバージョンを取得する機能。
 */
 
#ifndef __SPRESENSE_LTEMODEMVERIFICATION_H__
#define __SPRESENSE_LTEMODEMVERIFICATION_H__

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

#include <LTEDefinition.h>

/****************************************************************************
 * class declaration
 ****************************************************************************/

/**
 * @class LTEModemVerification
 *
 * @brief [en] The class to get modem information. <BR>
 *        [ja] モデムの情報を取得するクラス。
 *
 * @attention [en] To use this class, include LTE.h.
 *
 * @attention [ja] このクラスを使用する場合、 LTE.h をインクルードしてください。
 */

class LTEModemVerification{
public:

  /**
   * @brief Construct LTEModemVerification instance.
   */
  LTEModemVerification();

  /**
   * @brief Destruct LTEModemVerification instance.
   */
  ~LTEModemVerification();

  /**
   * @brief Power on the modem.
   *
   * @details [en] Power on the modem. If the modem is powered on, restart it.
   *               This method must be called before use any other methods.
   *
   * @details [ja] モデムの電源をONにします。モデムの電源がONの場合はモデムの電源をOFFにしてから電源をONにします。
   *               このメソッドはほかのメソッドを利用する前に必ず呼び出す必要があります。
   *
   * @return [en] Returns LTE_IDLE on success, LTE_ERROR if an error occurs.
   *
   * @return [ja] 成功時はLTE_IDLE、エラーが発生した場合、LTE_ERRORを返します。
   */
  LTEModemStatus begin();

  /**
   * @brief Get IMEI.
   *
   * @details [en] Get IMEI(International Mobile Equipment Identity) of modem.
   *
   * @details [ja] モデムのIMEI(International Mobile Equipment Identity)を取得します。
   *
   * @return [en] Returns IMEI of the modem on success, "N/A" if an error occurs.
   *
   * @return [ja] 成功時はモデムのIMEIを返し、エラーが発生した場合、"N/A"を返します。
   */
  String getIMEI();

  /**
   * @brief Get firmware version of the modem.
   */

  /**
   * @brief Get firmware version.
   *
   * @details [en] Get firmware version of the modem.
   *
   * @details [ja] モデムのファームウェアバージョンを取得します。
   *
   * @return [en] Returns firmware version of the modem on success, "N/A" if an error occurs.
   *
   * @return [ja] 成功時はモデムのファームウェアバージョンを返し、エラーが発生した場合、"N/A"を返します。
   */
  String getFirmwareVersion();

  /**
   * @brief Get RAT(Radio Access Technology).
   *
   * @details [en] Get the RAT(Radio Access Technology) currently used by the modem.
   *
   * @details [ja] モデムが現在使用しているRAT(Radio Access Technology)を取得します。
   *
   * @return [en] RAT type defined as #LTENetworkRatType. Returns LTE_NET_RAT_UNKNOWN if an error occurs.
   *
   * @return [ja] 成功時は #LTENetworkRatType が定義するRAT形式を返し、エラーが発生した場合、LTE_NET_RAT_UNKNOWNを返します。
   */

  LTENetworkRatType getRAT();

  /**
   * @brief Get the modem status.
   *
   * @details [en] Get the modem status.
   *
   * @details [ja] モデムの状態を取得します。
   *
   * @return [en] Status code defined as #LTEModemStatus.
   *
   * @return [ja] #LTEModemStatus で定義するステータスコード。
   */
  LTEModemStatus getStatus();
};

/** @} */

#endif // __SPRESENSE_LTEMODEMVERIFICATION_H__
