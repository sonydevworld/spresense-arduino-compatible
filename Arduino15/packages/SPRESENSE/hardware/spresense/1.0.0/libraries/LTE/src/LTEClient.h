/*
 *  LTEClient.h - LTEClient include file for Spresense Arduino
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
 * @file LTEClient.h
 *
 * @author Sony Semiconductor Solutions Corporation
 *
 * @brief LTE Client Library for Spresense Arduino.
 *
 * @details [en] By using this library, you can connect to servers and send and receive data.
 *
 * @details [ja] このライブラリを使用することで、サーバーに接続してデータを送受信できます。
 */

#ifndef _LTE_CLIENT_H_
#define _LTE_CLIENT_H_

#ifdef SUBCORE
#error "LTEClient library is NOT supported by SubCore."
#endif

/**
 * @defgroup lteclient LTE Client Library API
 *
 * @brief API for using LTE Client
 * @{
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <Client.h>

class IPAddress;

/****************************************************************************
 * class declaration
 ****************************************************************************/

/**
 * @class LTEClient
 *
 * @brief [en] Create a client that can connect to a specific Internet IP address and port. <BR>
 *        [ja] 特定のインターネットIPアドレスとポートに接続可能なクライアントを作成します。
 *
 */
class LTEClient : public Client
{
public:
  /**
   * @brief Construct LTEClient instance.
   */
  LTEClient();

  /**
   * @brief Destruct LTEClient instance.
   */
  ~LTEClient();

  /**
   * @brief Connects to a specified IP address and port.
   *
   * @details [en] Connects to a specified IP address and port.
   * @details [ja] 指定されたIPアドレスとポートに接続します。
   *
   * @param [in] ip [en] Server IP address. <BR>
   *                [ja] 接続先サーバーのIPアドレス。
   * @param [in] port [en] Server port. <BR>
   *                  [ja] 接続先サーバーのポート番号。
   *
   * @return [en] On success, 1 is returned. On failure, 0 is returned.
   *
   * @return [ja] 接続に成功した場合1を、失敗した場合は0を返します。
   */
  int connect(IPAddress ip, uint16_t port);

  /**
   * @brief Connects to a specified host name and port.
   *
   * @details [en] Connects to a specified host and port.
   *
   * @details [ja] 指定されたホスト名とポートに接続します。
   *
   * @param [in] host [en] Server host name. <BR>
   *                  [ja] 接続先サーバーのホスト名。
   * @param [in] port [en] Server port. <BR>
   *                  [ja] 接続先サーバーのポート番号。
   *
   * @return [en] On success, 1 is returned. On failure, 0 is returned.
   *
   * @return [ja] 接続に成功した場合1を、失敗した場合は0を返します。
   */
  int connect(const char *host, uint16_t port);

  /**
   * @brief Send one byte data to the server the client is connected to.
   *
   * @details [en] Send one byte data to the server the client is connected to.
   *
   * @details [ja] 接続先のサーバーに1バイトのデータを送信します。
   *
   * @param [in] val [en] A value to send as a single byte. <BR>
   *                 [ja] 送信する値。
   *
   * @return [en] The number of bytes sent.
   *
   * @return [ja] 送信されたバイト数。
   */
  size_t write(uint8_t val);

  /**
   * @brief Send series of bytes data to the server the client is connected to.
   *
   * @details [en] Send series of bytes data to the server the client is connected to.
   *
   * @details [ja] 接続先のサーバーに一連のデータを送信します。
   *
   * @param [in] buf [en] A buffer to send. <BR>
   *                 [ja] 送信バッファー。
   * @param [in] size [en] The length of the buffer.  <BR>
   *                  [ja] 送信バッファーの長さ。
   *
   * @return [en] The number of bytes sent.
   *
   * @return [ja] 送信されたバイト数。
   */
  size_t write(const uint8_t *buf, size_t size);

  /**
   * @brief Returns the number of bytes available for reading.
   *
   * @details [en] Returns the number of bytes available for reading.
   *
   * @details [ja] 読み出し可能なバイト数を返します。
   *
   * @return [en] The number of bytes available.
   *
   * @return [ja] 読出し可能なバイト数。
   */
  int available();

  /**
   * @brief Read the next byte received from the server the client is connected to.
   *
   * @details [en] Read the next byte received from the server the client is connected to.
   *
   * @details [ja] 接続先のサーバーから受信したデータを読み出します。
   *
   * @return [en] The next byte, or -1 if none is available.
   *
   * @return [ja] 受信したデータ。受信データがない場合は-1を返します。
   */
  int read();

  /**
   * @brief Read series of bytes received from the server the client is connected to.
   *
   * @details [en] Read series of bytes received from the server the client is connected to.
   *
   * @details [ja] 接続先のサーバーから受信した一連のデータを読み出します。
   *
   * @param [out] buf [en] A buffer to read. <BR>
   *                  [ja] 受信バッファー。
   * @param [in] size [en] The length of the buffer. <BR>
   *                  [ja] 受信バッファーの長さ。
   *
   * @return [en] The number of bytes received, or -1 if none is available.
   *
   * @return [ja] 受信したバイト数。受信データがない場合は-1を返します。
   */
  int read(uint8_t *buf, size_t size);

  /**
   * @brief Returns the next byte received from the server without removing it from the buffer.
   *
   * @details [en] Returns the next byte received from the server without removing it from the buffer. <BR>
   *
   * @details [ja] 接続先のサーバーから受信したデータを、バッファーから削除せずに読み出します。
   *
   * @return [en] The next byte, or -1 if none is available.
   *
   * @return [ja] 受信したデータ。受信データがない場合は-1を返します。
   */
  int peek();

  /**
   * @brief Discards any bytes that have been written to the client.
   *
   * @details [en] Discards any bytes that have been written to the client.
   *
   * @details [ja] クライアントに書き込まれたデータを破棄します。
   */
  void flush();

  /**
   * @brief Disconnect from the server.
   *
   * @details [en] Disconnect from the server.
   *
   * @details [ja] サーバーから切断します。
   */
  void stop();

  /**
   * @brief Whether or not the client is connected.
   *
   * @details [en] Whether or not the client is connected. Note that a client is considered connected if the connection has been closed but there is still unread data.
   *
   * @details [ja] クライアントが接続されているかどうかを返します。接続は閉じられているが未読データが残っている場合、クライアントは接続されていると見なされることに注意してください。
   *
   * @return [en] Returns 1 if the client is connected, 0 if not.
   *
   * @return [ja] クライアントが接続されている場合は1を、そうでない場合は0を返します。
   */
  uint8_t connected();

  /**
   * @brief Assignment operator for bool.
   *
   * @details [en] Whether or not the client is connected. 
   *
   * @details [ja] クライアントが接続されているかどうかを返します。
   *
   * @return [en] Returns 1 if the client is connected, 0 if not.
   *
   * @return [ja] クライアントが接続されている場合は1を、そうでない場合は0を返します。
   */
  operator bool()
    {
      return connected();
    };

  /**
   * @brief Set the timeout when the client send or receive.
   *
   * @details [en] Set the timeout when the client send or receive. 0 means disabled (no timeout). If this method has not been called, the timeout is 0.
   *
   * @details [ja] クライアントが送受信をする際のタイムアウトを設定します。0は無効（タイムアウトしない）を意味します。本メソッドを呼び出さない場合のタイムアウトは0です
   *
   * @return [en] Returns 0 if succeeded, -1 if not.
   *
   * @return [ja] 成功した場合は0を、そうでない場合は-1を返します。
   */
  int setTimeout(uint32_t milliseconds);

private:
  int _fd;
  uint8_t *_buf;
  uint8_t _connected;
};

/** @} lteclient */


#endif
