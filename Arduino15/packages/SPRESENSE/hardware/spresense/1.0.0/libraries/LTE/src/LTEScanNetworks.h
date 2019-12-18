/*
 *  LTEScanNetworks.h - LTEScanNetworks include file for Spresense Arduino
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
 * @file LTEScanNetworks.h
 *
 * @author Sony Semiconductor Solutions Corporation
 *
 * @brief Network information management class for Spresense Arduino.
 *
 * @attention [en] Use LTE.h instead of including this header file directly.
 *
 * @attention [ja] このヘッダファイルを直接インクルードせずに、 LTE.h を使用してください。
 *
 * @details [en] By using this class, you can use the follow features on SPRESSENSE.
 *           - Feature to get received signal strength of the LTE network.
 *           - Feature to get the name of the connected LTE network carrier.
 *
 * @details [ja] このクラスを使用することで、以下の機能をSPRESENSE上で利用することが出来ます。
 *           - LTEネットワークの受信強度を取得する機能。
 *           - 接続したLTEネットワークキャリア名を取得する機能。
 */

#ifndef __SPRESENSE_LTESCANNETWORKS_H__
#define __SPRESENSE_LTESCANNETWORKS_H__

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

class String;

/****************************************************************************
 * class declaration
 ****************************************************************************/

/**
 * @class LTEScanNetworks
 *
 * @brief [en] The class to get LTE network information.<BR>
 *        [ja] LTEネットワークの情報を取得するためのクラス。
 *
 * @attention [en] To use this class, include LTE.h.
 *
 * @attention [ja] このクラスを使用する場合、 LTE.h をインクルードしてください。
 */

class LTEScanNetworks{
public:

  /**
   * @brief Construct LTEScanNetworks instance.
   */
  LTEScanNetworks();

  /**
   * @brief Destruct LTEScanNetworks instance.
   */
  ~LTEScanNetworks();

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
   * @brief Get signal strength.
   *
   * @details [en] Get received signal strength of the LTE network.
   *
   * @details [ja] LTEネットワークの受信強度を取得します。
   *
   * @attention [en] The received signal strength cannot be obtained when the modem status is OFF or IDLE.
   *
   * @attention [ja] モデムの状態がOFFまたはIDLEの場合は受信強度を取得出来ません。
   *
   * @return [en] Returns RSSI(Received Signal Strength Indication)[dBm] on success,
   *              "N/A" if the received strength could not be obtained or an error occurs.
   *
   * @return [ja] 成功時はRSSI(Received Signal Strength Indication)[dBm]を返し、
   *              受信強度が取得できなかった場合、またはエラーが発生した場合、"N/A"を返します。
   */
  String getSignalStrength();

  /**
   * @brief Get the name of the connected LTE network carrier.
   *
   * @details [en] Get the name of the connected LTE network carrier.
   *
   * @details [ja] 接続したLTEネットワークキャリア名を取得します。
   *
   * @attention [en] The carrier name cannot be obtained when the modem status is not LTE_READY.
   *
   * @attention [ja] モデムの状態がLTE_READY状態でない場合はキャリア名を取得出来ません。
   *
   * @return [en] Returns the name of the connected LTE network carrier on success,
   *              "N/A" if the received LTE network carrier could not be obtained or an error occurs.
   *
   * @return [ja] 成功時は接続したLTEネットワークキャリア名を返し、
   *              LTEネットワークキャリア名が取得出来なかった場合、またはエラーが発生した場合、"N/A"を返します。
   */
  String getCurrentCarrier();

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

#endif // __SPRESENSE_LTESCANNETWORKS_H__
