/*
 *  LTEAccessProvider.h - LTEAccessProvider include file for Spresense Arduino
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
 * @file LTEAccessProvider.h
 *
 * @author Sony Semiconductor Solutions Corporation
 *
 * @brief LTE connection control class for Spresense Arduino.
 *
 * @attention [en] Use LTE.h instead of including this header file directly.
 *            [ja] このヘッダファイルを直接インクルードせずに、 LTE.h を使用してください。
 *
 * @details [en] By using this class, you can use the follow features on SPRESSENSE.
 *           - Feature of registering modem to LTE network.
 *           - Feature of detaching modem from LTE network.
 *           - Feature to get IP address assigned by LTE network.
 *           - Feature to get time.
 *
 * @details [ja] このクラスを使用することで、以下の機能をSPRESENSE上で利用することが出来ます。
 *           - LTEネットワークにモデムを登録する機能。
 *           - LTEネットワークからモデムを切り離す機能。
 *           - LTEネットワークが割り当てたIPアドレスを取得する機能。
 *           - 時刻を取得する機能。
 */

#ifndef __SPRESENSE_LTEACCESSPROVIDER_H__
#define __SPRESENSE_LTEACCESSPROVIDER_H__

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

class IPAddress;

/****************************************************************************
 * class declaration
 ****************************************************************************/

/**
 * @class LTEAccessProvider
 *
 * @brief [en] The class to construct a path for communication between
 *             the LTE network and modem. <BR>
 *        [ja] LTEネットワークとモデム間で通信するための通信経路を構築するクラス。
 *
 * @attention [en] To use this class, include LTE.h.
 *
 * @attention [ja] このクラスを使用する場合、 LTE.h をインクルードしてください。
 */
class LTEAccessProvider{
public:

  /**
   * @brief Construct LTEAccessProvider instance.
   */
  LTEAccessProvider();

  /**
   * @brief Destruct LTEAccessProvider instance.
   */
  ~LTEAccessProvider();

  /**
   * @brief Power on the modem and start the network search.
   *
   * @details [en] Power on the modem and search for the LTE network. If the restart flag is true,
   *               restart the modem and search for the LTE network. This method must be called before use any other methods.
   *
   * @details [ja] モデムの電源をONにし、LTEネットワークをサーチします。
   *               再起動フラグがtrueの場合、モデムの電源をOFFにしてから電源をONにし、LTEネットワークをサーチします。
   *               このメソッドはほかのメソッドを利用する前に必ず呼び出す必要があります。
   *
   * @param [in] pinCode [en] PIN unlock code. If PIN lock is disabled, set this parameter to NULL. <BR>
   *                     [ja] PINロック解除コード。PINロックが無効な場合、NULLを設定してください。
   * @param [in] restart [en] Restart flag <BR> [ja] 再起動フラグ
   * @param [in] synchronous [en] Synchronization wait flag <BR> [ja] 同期待ちフラグ
   *
   * @attention [en] If you enter the PIN code, check the operation on the serial console in advance.
   *                 If the PIN code is incorrect, an error message will be displayed with the remaining try count that can be executed.
   *                 If you have locked your modem beyond the remaining try count, please refer to the SIM manual to find out how to unlock it.
   *
   * @attention [ja] PINロック解除コードを入力した場合は、事前にシリアルコンソールでの動作確認を行ってください。
   *                 PINロック解除コードが間違っている場合は残り施行可能回数と共にエラーメッセージが出力されます。
   *                 残り施行可能回数を超えてロックされてしまった場合はSIMの説明書を参照して解除方法を確認してください。

   * @return [en] Returns LTE_SEARCHING on success, LTE_ERROR if an error occurs.
   *
   * @return [ja] 成功時はLTE_SEARCHING、エラーが発生した場合、LTE_ERRORを返します。
   */
  LTEModemStatus begin(char* pinCode = NULL, bool restart = true, bool synchronous = true);

  /**
   * @brief Power off the LTE modem and detach the modem from the LTE network.
   *
   * @details [en] Power off the LTE modem and detach the modem from the LTE network.
   *
   * @details [ja] モデムの電源をOFFにし、LTEネットワークからモデムを切り離します。
   */
  void shutdown();

  /**
   * @brief Register the modem on the LTE network.
   *
   * @details [en] Register the modem on the LTE network.
   *               If the synchronous parameter is false, please check the modem
   *               has been registered on the LTE network using the getStatus() method. <BR>
   *               The RAT used in this method will be the one previously configured on the modem.
   *               If you want to connect to the LTE network by specifying RAT, use the attach() method described later.
   *
   * @details [ja] LTEネットワークにモデムを登録します。
   *               synchronousパラメータがfalseの場合、getStatus()メソッドを使用して、
   *               LTEネットワークにモデムが登録できたことを確認してください。 <BR>
   *               このメソッドで使用されるRATは、以前モデムに設定されたものになります。
   *               RATを指定してLTEネットワークに接続したい場合は、後述するattach()メソッドを使用してください。
   *
   * @param [in] apn [en] Access point name
   *                 [ja] アクセスポイント名
   * @param [in] userName [en] Authentication user name. If authentication is not required, set this parameter to NULL. <BR>
   *                      [ja] 認証ユーザ名。ユーザ認証が必要ない場合、NULLを設定してください。
   * @param [in] password [en] Authentication password. If authentication is not required, set this parameter to NULL. <BR>
   *                      [ja] 認証パスワード。ユーザ認証が必要ない場合、NULLを設定してください。
   * @param [in] authType [en] Authentication type. Set the value defined in #LTENetworkAuthType. <BR>
   *                      [ja] 認証形式。 #LTENetworkAuthType で定義された値を設定してください。
   * @param [in] ipType [en] Connection IP type. Set the value defined in #LTENetworkIPType. <BR>
   *                    [ja] 接続IP形式。 #LTENetworkIPType で定義された値を設定してください。
   * @param [in] synchronous [en] Synchronization wait flag <BR> [ja] 同期待ちフラグ
   *
   * @attention [en] If rejected from the LTE network, the status changes to LTE_ERROR.
   *
   * @attention [ja] LTEネットワークからリジェクトされた場合、ステータスはLTE_ERRORに遷移します。
   *
   * @return [en] The return value on success depends on the value of synchronous.
   *         - If synchronous is true <BR>
   *          Returns LTE_READY on success, LTE_ERROR if an error occurs.
   *         - If synchronous is false <BR>
   *          Returns LTE_CONNECTING on success, LTE_ERROR if an error occurs.
   *
   * @return [ja] 成功時の戻り値は、synchronousパラメータの値によって変わります。
   *         - synchronousの値がtrueの場合 <BR>
   *          成功時はLTE_READY、エラーが発生した場合、LTE_ERRORを返します。
   *         - synchronousの値がfalseの場合 <BR>
   *          成功時はLTE_CONNECTING、エラーが発生した場合、LTE_ERRORを返します。
   */
  LTEModemStatus attach(const char *apn,
                        const char *userName = NULL,
                        const char *password = NULL,
                        LTENetworkAuthType authType = LTE_NET_AUTHTYPE_CHAP,
                        LTENetworkIPType ipType = LTE_NET_IPTYPE_V4V6,
                        bool synchronous = true);

  /**
   * @brief Register the modem on the LTE network after configuring RAT.
   *
   * @details [en] Registers the modem with the LTE network for the specified RAT.
   *               RAT can specify LTE-M (LTE Cat-M1) / NB-IoT depending on the SIM contract you are using.
   *               Please check your SIM contract and specify RAT. <BR>
   *               If the synchronous parameter is false, please check the modem
   *               has been registered on the LTE network using the getStatus() method.
   *
   * @details [ja] 指定されたRATのLTEネットワークにモデムを登録します。
   *               RATは使用しているSIM契約に応じて、LTE-M (LTE Cat-M1)/NB-IoTを指定できます。
   *               ご使用のSIMの契約を確認し、RATを指定してください。<BR>
   *               synchronousパラメータがfalseの場合、getStatus()メソッドを使用して、
   *               LTEネットワークにモデムが登録できたことを確認してください。
   *
   * @param [in] rat [en] RAT(Radio Access Technology). Set the value defined in #LTENetworkRatType. <BR>
   *                 [ja] RAT(Radio Access Technology)。 #LTENetworkRatType で定義された値を設定してください。
   * @param [in] apn [en] Access point name
   *                 [ja] アクセスポイント名
   * @param [in] userName [en] Authentication user name. If authentication is not required, set this parameter to NULL. <BR>
   *                      [ja] 認証ユーザ名。ユーザ認証が必要ない場合、NULLを設定してください。
   * @param [in] password [en] Authentication password. If authentication is not required, set this parameter to NULL. <BR>
   *                      [ja] 認証パスワード。ユーザ認証が必要ない場合、NULLを設定してください。
   * @param [in] authType [en] Authentication type. Set the value defined in #LTENetworkAuthType. <BR>
   *                      [ja] 認証形式。 #LTENetworkAuthType で定義された値を設定してください。
   * @param [in] ipType [en] Connection IP type. Set the value defined in #LTENetworkIPType. <BR>
   *                    [ja] 接続IP形式。 #LTENetworkIPType で定義された値を設定してください。
   * @param [in] synchronous [en] Synchronization wait flag <BR> [ja] 同期待ちフラグ
   *
   * @attention [en] If rejected from the LTE network, the status changes to LTE_ERROR.
   *
   * @attention [ja] LTEネットワークからリジェクトされた場合、ステータスはLTE_ERRORに遷移します。
   *
   * @return [en] The return value on success depends on the value of synchronous.
   *         - If synchronous is true <BR>
   *          Returns LTE_READY on success, LTE_ERROR if an error occurs.
   *         - If synchronous is false <BR>
   *          Returns LTE_CONNECTING on success, LTE_ERROR if an error occurs.
   *
   * @return [ja] 成功時の戻り値は、synchronousパラメータの値によって変わります。
   *         - synchronousの値がtrueの場合 <BR>
   *          成功時はLTE_READY、エラーが発生した場合、LTE_ERRORを返します。
   *         - synchronousの値がfalseの場合 <BR>
   *          成功時はLTE_CONNECTING、エラーが発生した場合、LTE_ERRORを返します。
   */
  LTEModemStatus attach(LTENetworkRatType rat,
                        const char *apn,
                        const char *userName = NULL,
                        const char *password = NULL,
                        LTENetworkAuthType authType = LTE_NET_AUTHTYPE_CHAP,
                        LTENetworkIPType ipType = LTE_NET_IPTYPE_V4V6, 
                        bool synchronous = true);

  /**
   * @brief Detach the modem from the LTE network.
   *
   * @details [en] Detach the modem from the LTE network. If this method is called during
   *               the asynchronous execution of attach(), modem registration processing is canceled.
   *
   * @details [ja] LTEネットワークからモデムを切り離します。attach()を非同期実行中に本メソッドが呼ばれた場合、
   *               モデムの登録処理をキャンセルします。
   *
   * @attention [en] If this method is called when the modem status is LTE_CONNECTING, LTE_READY may be returned in conflict with the LTE network registration process.
   *                 When detaching from the LTE network, please execute detach() again.
   *
   * @attention [ja] モデムの状態がLTE_CONNECTINGの時に本メソッドを呼んだ場合、LTEネットワークへの登録処理と競合して、LTE_READYが返る場合があります。
   *                 LTEネットワークから切り離す場合、 detach() をもう一度実行してください。
   *
   * @retval LTE_SEARCHING [en] Returns when the modem is detached from the LTE network.<BR>
   *                       [ja] LTEネットワークからモデムが切り離された場合に返す。
   *
   * @retval LTE_READY [en] Returns when the modem is registered to the LTE network by calling this method if the modem status is LTE_CONNECTING.<BR>
   *                   [ja] モデムの状態がLTE_CONNECTINGの時に本メソッドを呼び出し、LTEネットワークにモデムが登録された場合に返す。
   *
   * @retval LTE_ERROR [en] Returns when an error occurred.<BR>
   *                   [ja] エラーが発生した場合に返す。
   */
  LTEModemStatus detach();

  /**
   * @brief Get IP address assigned by LTE network.
   *
   * @details [en] Get IP address assigned by LTE network.
   *
   * @details [ja] LTEネットワークが割り当てたIPアドレスを取得します。
   *
   * @attention [en] The IP address cannot be obtained unless the modem status is LTE_READY.
   *                 The only IP address that can be obtained with this method is IPv4.<BR>
   *                 IP address may not be acquired immediately after the LTE_READY state transition.
   *                 Please execute this API after waiting for 1 second or more after
   *                 transitioning to the LTE_READY state.
   *
   * @attention [ja] モデムがLTE_READY状態でないとIPアドレスは取得出来ません。
   *                 このメソッドで取得できるIPアドレスはIPv4のみです。<BR>
   *                 LTE_READY状態遷移直後はIPアドレスが取得できない場合があります。
   *                 LTE_READY状態に遷移してから1秒以上待機してから本APIを実行してください。
   *
   * @return [en] Returns IP address on success, empty object if an error occurs.
   *
   * @return [ja] 成功時はIP address、エラーが発生した場合、空のIPAddressオブジェクトを返します。
   */
  IPAddress getIPAddress();

  /**
   * @brief Gets the number of seconds since the epoch.
   *
   * @details [en] Gets the number of seconds since the epoch (1970-01-01 00:00:00 (UTC)).
   *
   * @details [ja] 紀元 (Epoch; 1970-01-01 00:00:00 (UTC)) からの秒数を取得します。
   *
   * @attention [en] The time cannot be obtained when the modem status is not LTE_READY.
   *
   * @attention [ja] モデムの状態がLTE_READY状態でない場合は時刻を取得出来ません。
   *
   * @return [en] Returns the time in seconds since the epoch (1970-01-01 00:00:00 (UTC)) on success,
   *              0 if an error occurs.
   *
   * @return [ja] 成功時は紀元 (Epoch; 1970-01-01 00:00:00 (UTC)) からの秒数を返し、
   *              エラーが発生した場合、0を返します。
   */
  unsigned long getTime();

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

#endif // __SPRESENSE_LTEACCESSPROVIDER_H__
