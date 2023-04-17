/*
 *  LTETLSClient.h - LTETLSClient include file for Spresense Arduino
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
 * @file LTETLSClient.h
 *
 * @author Sony Semiconductor Solutions Corporation
 *
 * @brief LTE Secure Client Library for Spresense Arduino.
 *
 * @details [en] By using this library, you can connect to servers and send and receive data securely.
 *
 * @details [ja] このライブラリを使用することで、サーバーに接続してデータをセキュアーに送受信できます。
 */

#ifndef _LTE_TLS_CLIENT_H_
#define _LTE_TLS_CLIENT_H_

#ifdef SUBCORE
#error "LTETLSClient library is NOT supported by SubCore."
#endif

/**
 * @defgroup ltetlsclient LTE Secure Client Library API
 *
 * @brief API for using LTE Secure Client
 * @{
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <Client.h>
#include <TLSClient.h>
#include <File.h>

/****************************************************************************
 * class declaration
 ****************************************************************************/

/**
 * @class LTETLSClient
 *
 * @brief [en] Create a secure client that can connect to a specific Internet IP address and port. <BR>
 *        [ja] 特定のインターネットIPアドレスとポートに接続可能なセキュアーなクライアントを作成します。
 *
 */
class LTETLSClient : public Client
{
public:
  /**
   * @brief Construct LTETLSClient instance.
   */
  LTETLSClient();

  /**
   * @brief Destruct LTETLSClient instance.
   */
  ~LTETLSClient();

  /**
   * @brief Connects to a specified IP address and port.
   *
   * @details [en] Connects to a specified IP address and port.
   *
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
   * @details [en] Returns the next byte received from the server without removing it from the buffer.
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
   * @brief Set the root certificate authority certificate on the client.
   *
   * @details [en] Set the root certificate authority certificate in PEM format on the client. Please call this method before connecting to the server by connect() method.
   *
   * @details [ja] PEM形式のルート認証局の証明書をクライアントに設定します。connect()のメソッドでサーバーに接続する前に本メソッドを呼び出してください。
   *
   * @param [in] rootCA [en] An array to set the root CA certificate as a series of characters. <BR>
   *                    [ja] ルート認証局の証明書の文字列。
   */
  void setCACert(const char *rootCA);

  /**
   * @brief Set the root certificate authority certificate on the client.
   *
   * @details [en] Set the root certificate authority certificate in DER format on the client. Please call this method before connecting to the server by connect() method.
   *
   * @details [ja] DER形式のルート認証局の証明書をクライアントに設定します。connect()のメソッドでサーバーに接続する前に本メソッドを呼び出してください。
   *
   * @param [in] rootCA [en] An array to set the root CA certificate as a series of bytes. <BR>
   *                    [ja] ルート認証局の証明書のバイト配列。
   * @param [in] size [en] Size of root certificate authority certificate. <BR>
   *                  [ja] ルート認証局の証明書のサイズ。
   */
  void setCACert(const unsigned char *rootCA, size_t size);

  /**
   * @brief Read the root certificate authority certificate from the file and set it on the client.
   *
   * @details [en] Read the root certificate authority certificate from the file and set it on the client. Both DER and PEM formats can be set. Please call this method before connecting to the server by connect() method.
   *
   * @details [ja] ルート認証局の証明書をファイルから読み出してクライアントに設定します。DER形式とPEM形式のどちらも設定可能です。connect()のメソッドでサーバーに接続する前に本メソッドを呼び出してください。
   *
   * @param [in] f [en] File to set the root CA certificate. <BR>
   *                    [ja] ルート認証局の証明書のファイルオブジェクト。
   * @param [in] size [en] Readable size. <BR>
   *                  [ja] 読み出し可能なサイズ。
   */
  void setCACert(File& f, size_t size);

  /**
   * @brief Read the root certificate authority certificate from the file or other stream and set it on the client.
   *
   * @details [en] Read the root certificate authority certificate from the file or other stream and set it on the client. Both DER and PEM formats can be set. Please call this method before connecting to the server by connect() method.
   *
   * @details [ja] ルート認証局の証明書をファイルなどのストリームから読み出してクライアントに設定します。DER形式とPEM形式のどちらも設定可能です。connect()のメソッドでサーバーに接続する前に本メソッドを呼び出してください。
   *
   * @param [in] stream [en] Stream to set the root CA certificate. <BR>
   *                    [ja] ルート認証局の証明書のストリーム。
   * @param [in] size [en] Readable size. <BR>
   *                  [ja] 読み出し可能なサイズ。
   */
  void setCACert(Stream& stream, size_t size);

  /**
   * @brief Set the certificate on the client.
   *
   * @details [en] Set the certificate in PEM format on the client. Please call this method before connecting to the server by connect() method if you need.
   *
   * @details [ja] PEM形式のクライアント証明書をクライアントに設定します。必要であれば、connect()のメソッドでサーバーに接続する前に本メソッドを呼び出してください。
   *
   * @param [in] clientCA [en] An array to set the certificate as a series of characters. <BR>
   *                      [ja] クライアント証明書の文字列。
   */
  void setCertificate(const char *clientCA);

  /**
   * @brief Set the certificate on the client.
   *
   * @details [en] Set the certificate in DER format on the client. Please call this method before connecting to the server by connect() method if you need.
   *
   * @details [ja] DER形式のクライアント証明書をクライアントに設定します。必要であれば、connect()のメソッドでサーバーに接続する前に本メソッドを呼び出してください。
   *
   * @param [in] clientCA [en] An array to set the certificate as a series of bytes. <BR>
   *                      [ja] クライアント証明書のバイト配列。
   * @param [in] size [en] Size of certificate. <BR>
   *                  [ja] クライアント証明書のサイズ。
   */
  void setCertificate(const unsigned char *clientCA, size_t size);

  /**
   * @brief Read the certificate from the file and set it on the client.
   *
   * @details [en] Read the certificate from the file and set it on the client. Both DER and PEM formats can be set. Please call this method before connecting to the server by connect() method if you need.
   *
   * @details [ja] クライアント証明書をファイルから読み出してクライアントに設定します。DER形式とPEM形式のどちらも設定可能です。必要であれば、connect()のメソッドでサーバーに接続する前に本メソッドを呼び出してください。
   *
   * @param [in] f [en] File to set the certificate. <BR>
   *                    [ja] クライアント証明書のファイルオブジェクト。
   * @param [in] size [en] Readable size. <BR>
   *                  [ja] 読み出し可能なサイズ。
   */
  void setCertificate(File& f, size_t size);

  /**
   * @brief Read the certificate from the file or other stream and set it on the client.
   *
   * @details [en] Read the certificate from the file or other stream and set it on the client. Both DER and PEM formats can be set. Please call this method before connecting to the server by connect() method if you need.
   *
   * @details [ja] クライアント証明書をファイルなどのストリームから読み出してクライアントに設定します。DER形式とPEM形式のどちらも設定可能です。必要であれば、connect()のメソッドでサーバーに接続する前に本メソッドを呼び出してください。
   *
   * @param [in] stream [en] Stream to set the certificate. <BR>
   *                    [ja] クライアント証明書のストリーム。
   * @param [in] size [en] Readable size. <BR>
   *                  [ja] 読み出し可能なサイズ。
   */
  void setCertificate(Stream& stream, size_t size);

  /**
   * @brief Set the private key on the client.
   *
   * @details [en] Set the private key in PEM format on the client. Please call this method before connecting to the server by connect() method if you need.
   *
   * @details [ja] PEM形式の秘密鍵をクライアントに設定します。必要であれば、connect()のメソッドでサーバーに接続する前に本メソッドを呼び出してください。
   *
   * @param [in] privateKey [en] An array to set the private key as a series of characters. <BR>
   *                        [ja] 秘密鍵の文字列。
   */
  void setPrivateKey(const char *privateKey);

  /**
   * @brief Set the private key on the client.
   *
   * @details [en] Set the private key in DER format on the client. Please call this method before connecting to the server by connect() method if you need.
   *
   * @details [ja] DER形式の秘密鍵をクライアントに設定します。必要であれば、connect()のメソッドでサーバーに接続する前に本メソッドを呼び出してください。
   *
   * @param [in] privateKey [en] An array to set the private key as a series of bytes. <BR>
   *                        [ja] 秘密鍵のバイト配列。
   * @param [in] size [en] Size of private key. <BR>
   *                  [ja] 秘密鍵のサイズ。
   */
  void setPrivateKey(const unsigned char *privateKey, size_t size);

  /**
   * @brief Read the private key from the file and set it on the client.
   *
   * @details [en] Read the private key from the file and set it on the client. Both DER and PEM formats can be set. Please call this method before connecting to the server by connect() method if you need.
   *
   * @details [ja] 秘密鍵をファイルから読み出してクライアントに設定します。DER形式とPEM形式のどちらも設定可能です。必要であれば、connect()のメソッドでサーバーに接続する前に本メソッドを呼び出してください。
   *
   * @param [in] f [en] File to set the private key. <BR>
   *                    [ja] 秘密鍵のファイルオブジェクト。
   * @param [in] size [en] Readable size. <BR>
   *                  [ja] 読み出し可能なサイズ。
   */
  void setPrivateKey(File& f, size_t size);

  /**
   * @brief Read the private key from the file or other stream and set it on the client.
   *
   * @details [en] Read the private key from the file or other stream and set it on the client. Both DER and PEM formats can be set. Please call this method before connecting to the server by connect() method if you need.
   *
   * @details [ja] 秘密鍵をファイルなどのストリームから読み出してクライアントに設定します。DER形式とPEM形式のどちらも設定可能です。必要であれば、connect()のメソッドでサーバーに接続する前に本メソッドを呼び出してください。
   *
   * @param [in] stream [en] Stream to set the private key. <BR>
   *                    [ja] 秘密鍵のストリーム。
   * @param [in] size [en] Readable size. <BR>
   *                  [ja] 読み出し可能なサイズ。
   */
  void setPrivateKey(Stream& stream, size_t size);

  /**
   * @brief Set the timeout when the client receive (including TLS handshake).
   *
   * @details [en] Set the timeout when the client receive (including TLS handshake). 0 means disabled (no timeout). If this method has not been called, the timeout is 10 seconds.
   *
   * @details [ja] クライアントが受信をする際のタイムアウトを設定します(TLSハンドシェイクを含みます)。0は無効（タイムアウトしない）を意味します。本メソッドを呼び出さない場合のタイムアウトは10秒です。
   *
   * @return [en] Returns 0 if succeeded, -1 if not.
   *
   * @return [ja] 成功した場合は0を、そうでない場合は-1を返します。
   */
  int setTimeout(uint32_t milliseconds);

  /**
   * @brief Set the timeout when the client send.
   *
   * @details [en] Set the timeout when the client send. 0 means disabled (no timeout). If this method has not been called, the timeout is 60 seconds.
   *
   * @details [ja] クライアントが送信をする際のタイムアウトを設定します。0は無効（タイムアウトしない）を意味します。本メソッドを呼び出さない場合のタイムアウトは60秒です。
   *
   * @return [en] Returns 0 if succeeded, -1 if not.
   *
   * @return [ja] 成功した場合は0を、そうでない場合は-1を返します。
   */
  int setSendTimeout(uint32_t milliseconds);

private:
  int _peekVal;
  char *_rootCA;
  size_t _rootCASize;
  char *_clientCA;
  size_t _clientCASize;
  char *_privateKey;
  size_t _privateKeySize;
  tlsClientContext_t *_tlsContext;
  uint8_t _connected;
  uint32_t _timeout;
  uint32_t _writeTimeout;
};

/** @} ltetlsclient */


#endif
