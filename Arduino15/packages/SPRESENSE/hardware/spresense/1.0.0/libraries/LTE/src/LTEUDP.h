/*
 *  LTEUDP.h - LTEUDP include file for Spresense Arduino
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
 * @file LTEUDP.h
 *
 * @author Sony Semiconductor Solutions Corporation
 *
 * @brief LTE UDP Library for Spresense Arduino.
 *
 * @details [en] By using this library, you can UDP packet to be sent and received.
 *
 * @details [ja] このライブラリを使用することで、UDPパケットを送受信できます。
 */

#ifndef _LTE_UDP_H_
#define _LTE_UDP_H_

#ifdef SUBCORE
#error "LTEUDP library is NOT supported by SubCore."
#endif

/**
 * @defgroup lteudp LTE UDP Library API
 *
 * @brief API for using LTE UDP
 * @{
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <Udp.h>

class IPAddress;

/****************************************************************************
 * class declaration
 ****************************************************************************/

/**
 * @class LTEUDPBuffer
 *
 * @brief [en] UDP packet memory management class. This is internal use class. <BR>
 *        [ja] UDPパケットメモリ管理用クラス。内部利用Class。
 *
 */
class LTEUDPBuffer
{
public:
  LTEUDPBuffer();
  LTEUDPBuffer(size_t size);
  ~LTEUDPBuffer();

  size_t write(uint8_t val);
  size_t write(const uint8_t *buffer, size_t size);
  int available();
  int read();
  int read(char* buffer, size_t size);
  int peek();

private:
  size_t _maxSize;
  char *_buf;
  int _begin;
  int _end;
};

/**
 * @class LTEUDP
 * @brief [en] This class can send and receive UDP packets to a specific Internet IP address and port. <BR>
 *        [ja] 特定のインターネットIPアドレスとポートに対してUDPパケットの送受信ができます。
 *
 */
class LTEUDP : public UDP
{
public:
  /**
   * @brief Construct LTEUDP instance.
   */
  LTEUDP();

  /**
   * @brief Destruct LTEUDP instance.
   */
  ~LTEUDP();

  /**
   * @brief Initialize, start listening on specified port.
   *
   * @details [en] Initialize, start listening on specified port.
   *
   * @details [ja] クラスの初期化及び指定されたポートの接続を待ちます。
   *
   * @param [in] port [en] Local port. <BR>
   *                  [ja] ローカルポート番号。
   *
   * @return [en] Returns 1 if successful, 0 if there are no sockets available to use.
   *
   * @return [ja] 成功した場合は1、使用可能なソケットがない場合は0を返します。
   */
  uint8_t begin(uint16_t port);

  /**
   * @brief Finish with the UDP socket.
   *
   * @details [en] Finish with the UDP socket.
   *
   * @details [ja] UDPソケットを閉じて終了します。
   */
  void stop();

  /**
   * @brief Start building up a packet to send to the remote host specific in ip and port.
   *
   * @details [en] Start building up a packet to send to the remote host specific in ip and port.
   *
   * @details [ja] 指定されたIPアドレスおよびポート番号でリモートホストに送信するパケットの構築を開始します。
   *
   * @param [in] ip [en] Remote IP address. <BR>
   *                [ja] リモートIPアドレス。
   * @param [in] port [en] Remote port. <BR>
   *                  [ja] リモートポート番号。
   *
   * @return [en] Returns 1 if successful, 0 if there was a problem with the supplied IP address or port.
   *
   * @return [ja] 成功した場合は1、指定されたIPアドレスまたはポート番号に問題があった場合は0を返します。
   */
  int beginPacket(IPAddress ip, uint16_t port);

  /**
   * @brief Start building up a packet to send to the remote host specific in host and port.
   *
   * @details [en] Start building up a packet to send to the remote host specific in host and port.
   *
   * @details [ja] 指定されたホスト名およびポート番号でリモートホストに送信するパケットの構築を開始します。
   *
   * @param [in] host [en] Remote host. <BR>
   *                  [ja] リモートホスト名。
   * @param [in] port [en] Remote port. <BR>
   *                  [ja] リモートポート番号。
   * @return [en] Returns 1 if successful, 0 if there was a problem resolving the hostname or port.
   *
   * @return [ja] 成功した場合は1、指定されたホスト名またはポート番号に問題があった場合は0を返します。
   */
  int beginPacket(const char *host, uint16_t port);

  /**
   * @brief Finish off this packet and send it.
   *
   * @details [en] Finish off this packet and send it.
   *
   * @details [ja] このパケットを終了して構築したパケットを送信します。
   *
   * @return [en] Returns 1 if the packet was sent successfully, 0 if there was an error.
   *
   * @return [ja] パケットが正常に送信された場合は1、エラーがあった場合は0を返します。
   */
  int endPacket();

  /**
   * @brief Write a single byte into the packet.
   *
   * @details [en] Write a single byte into the packet.
   *
   * @details [ja] パケットに1バイトのデータを書き込みます。
   *
   * @param [in] val [en] single byte. <BR>
   *                 [ja] 書き込む値。
   *
   * @return [en] The number of bytes written.
   *
   * @return [ja] 書き込まれたバイト数。
   */
  size_t write(uint8_t val);

  /**
   * @brief Write series of bytes from buffer into the packet.
   *
   * @details [en] Write series of bytes from buffer into the packet.
   *
   * @details [ja] パケットに一連のデータを書き込みます。
   *
   * @param [in] buffer [en] A buffer to send. <BR>
   *                    [ja] 書き込みバッファー。
   * @param [in] size [en] The length of the buffer. <BR>
   *                  [ja] 書き込みバッファーの長さ。
   *
   * @return [en] The number of bytes written.
   *
   * @return [ja] 書き込まれたバイト数。
   */
  size_t write(const uint8_t *buffer, size_t size);

  /**
   * @brief Start processing the next available incoming packet.
   *
   * @details [en] Start processing the next available incoming packet.
   *
   * @details [ja] 次に利用可能な受信パケットの処理を開始します。
   *
   * @return [en] Returns the size of the packet in bytes, or 0 if no packets are available.
   *
   * @return [ja] パケットのサイズをバイト単位で返します。利用可能なパケットがない場合は0を返します。
   */
  int parsePacket();

  /**
   * @brief Returns number of bytes remaining in the current packet.
   *
   * @details [en] Returns number of bytes remaining in the current packet.
   *
   * @details [ja] パケットの残りバイト数を返します。
   *
   * @return [en] The number of bytes available.
   *
   * @return [ja] 読出し可能バイト数。
   */
  int available();

  /**
   * @brief Read a single byte from the current packet.
   *
   * @details [en] Read a single byte from the current packet.
   *
   * @details [ja] 現在のパケットから1バイトを読み出します。
   *
   * @return [en] The next byte, or -1 if none is available.
   *
   * @return [ja] 読み出した値。読み出し可能なものがない場合は-1を返します。
   */
  int read();

  /**
   * @brief Read up to len bytes from the current packet and place them into buffer.
   *
   * @details [en] Read up to len bytes from the current packet and place them into buffer.
   *
   * @details [ja] 現在のパケットから最大lenバイトをバッファーに読み出します。
   *
   * @param [out] buffer [en] A buffer to read. <BR>
   *                     [ja] 受信バッファー。
   * @param [in] len [en] The length of the buffer. <BR>
   *                 [ja] 受信バッファーの長さ。
   *
   * @return [en] Returns the number of bytes read, or -1 if none are available.
   *
   * @return [ja] 読み出したバイト数を返します。読み出し可能なものがない場合は-1を返します。
   */
  int read(unsigned char* buffer, size_t len);

  /**
   * @brief Read up to len characters from the current packet and place them into buffer.
   *
   * @details [en] Read up to len characters from the current packet and place them into buffer.
   *
   * @details [ja] 現在のパケットから最大len文字をバッファーに読み出します。
   *
   * @param [out] buffer [en] A buffer to read. <BR>
   *                     [ja] 受信バッファー。
   * @param [in] len [en] The length of the buffer. <BR>
   *                 [ja] 受信バッファーの長さ。
   *
   * @return [en] Returns the number of characters read, or -1 if none are available.
   *
   * @return [ja] 読み出した文字数を返します。読み出し可能なものがない場合は-1を返します。
   */
  int read(char* buffer, size_t len);

  /**
   * @brief Return the next byte from the current packet without removing it.
   *
   * @details [en] Return the next byte from the current packet without removing it.
   *
   * @details [ja] 現在のパケットから削除せずにデータを読み出します。
   *
   * @return [en] The next byte received, or -1 if none is available.
   *
   * @return [ja] 読み出したデータを返します。読み出し可能なものがない場合は-1を返します。
   */
  int peek();

  /**
   * @brief Discards any bytes that have been written to the packet.
   *
   * @details [en] Discards any bytes that have been written to the packet.
   *
   * @details [ja] パケットに書き込まれたデータを破棄します。
   */
  void flush();

  /**
   * @brief Return the IP address of the host who sent the current incoming packet.
   *
   * @details [en] Return the IP address of the host who sent the current incoming packet.
   *
   * @details [ja] 現在の受信パケットを送信したホストのIPアドレスを返します。
   *
   * @return [en] Remote IP address.
   *
   * @return [ja] リモートIPアドレス。
   */
  IPAddress remoteIP();

  /**
   * @brief Return the port of the host who sent the current incoming packet.
   *
   * @details [en] Return the port of the host who sent the current incoming packet.
   *
   * @details [ja] 現在の受信パケットを送信したホストのポート番号を返します。
   *
   * @return [en] Remote port.
   *
   * @return [ja] リモートポート番号。
   */
  uint16_t remotePort();

  /**
   * @brief Set the timeout when send or receive.
   *
   * @details [en] Set the timeout send or receive. 0 means disabled (no timeout). If this method has not been called, the timeout is 0.
   *
   * @details [ja] 送受信をする際のタイムアウトを設定します。0は無効（タイムアウトしない）を意味します。本メソッドを呼び出さない場合のタイムアウトは0です。
   *
   * @return [en] Returns 0 if succeeded, -1 if not.
   *
   * @return [ja] 成功した場合は0を、そうでない場合は-1を返します。
   */
  int setTimeout(uint32_t milliseconds);

private:
  int _fd;
  uint8_t *_wbuf;
  size_t _wbufSize;
  LTEUDPBuffer *_rbuf;
  IPAddress _remoteIp;
  uint16_t _remotePort;
};

/** @} lteudp */


#endif
