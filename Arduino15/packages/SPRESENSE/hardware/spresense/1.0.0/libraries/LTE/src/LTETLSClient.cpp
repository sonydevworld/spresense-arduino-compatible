/*
 *  LTETLSClient.cpp - LTETLSClient implementation file for Spresense Arduino
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
 * @file LTETLSClient.cpp
 *
 * @author Sony Semiconductor Solutions Corporation
 *
 * @brief LTE Secure Client Library for Spresense Arduino.
 *
 * @details [en] By using this library, you can connect to servers and send and receive data securely.
 *
 * @details [ja] このライブラリを使用することで、サーバーに接続してデータをセキュアーに送受信できます。
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <LTETLSClient.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef BRD_DEBUG
#define LTETLSCDBG(format, ...) ::printf("DEBUG:LTETLSClient:%d " format, __LINE__, ##__VA_ARGS__)
#else
#define LTETLSCDBG(format, ...)
#endif
#define LTETLSCERR(format, ...) ::printf("ERROR:LTETLSClient:%d " format, __LINE__, ##__VA_ARGS__)

#define NOT_CONNECTED  0
#define CONNECTED      1
#define FAILED         -1
#define TLS_READ_TIMEOUT 10000
#define TLS_WRITE_TIMEOUT (60*1000)

/****************************************************************************
 * Public Functions
 ****************************************************************************/

LTETLSClient::LTETLSClient()
: _peekVal(-1)
, _rootCA(NULL)
, _rootCASize(0)
, _clientCA(NULL)
, _clientCASize(0)
, _privateKey(NULL)
, _privateKeySize(0)
, _tlsContext(NULL)
, _connected(NOT_CONNECTED)
, _timeout(TLS_READ_TIMEOUT)
, _writeTimeout(TLS_WRITE_TIMEOUT)
{
}

LTETLSClient::~LTETLSClient()
{
  stop();
  if (_rootCA) {
    delete[] _rootCA;
    _rootCA = NULL;
  }
  if (_clientCA) {
    delete[] _clientCA;
    _clientCA = NULL;
  }
  if (_privateKey) {
    delete[] _privateKey;
    _privateKey = NULL;
  }
}

int LTETLSClient::connect(IPAddress ip, uint16_t port)
{
  struct in_addr addr;
  addr.s_addr = ip;

  return connect(inet_ntoa(addr), port);
}

int LTETLSClient::connect(const char *host, uint16_t port)
{
  int ret;

  if (!host) {
    LTETLSCERR("invalid parameter\n");
    return NOT_CONNECTED;
  }

  stop();

  _tlsContext = new tlsClientContext_t;
  if (!_tlsContext) {
    LTETLSCERR("failed to allocate memory\n");
    return NOT_CONNECTED;
  }
  tlsInit(_tlsContext);

  ret = tlsConnect(_tlsContext, host, port, _timeout, _rootCA, _rootCASize,
                   _clientCA, _clientCASize, _privateKey, _privateKeySize);
  if (ret < 0) {
    stop();
    return NOT_CONNECTED;
  }

  _connected = CONNECTED;

  LTETLSCDBG("connected to %s\n", host);

  return CONNECTED;
}

size_t LTETLSClient::write(uint8_t val)
{
  return write(&val, 1);
}

size_t LTETLSClient::write(const uint8_t *buf, size_t size)
{
  int ret;

  if (!buf || !size) {
    LTETLSCERR("invalid parameter\n");
    return 0;
  }

  if (!_connected) {
    LTETLSCERR("not connected\n");
    return 0;
  }

  ret = tlsWrite(_tlsContext, buf, size, _writeTimeout);
  if (ret < 0) {
    stop();
    return 0;
  }

  LTETLSCDBG("written %d byte\n", ret);

  return ret;
}

int LTETLSClient::available()
{
  int ret = 0;

  if (!_connected) {
    if (_peekVal >= 0) {
      ret += 1;
    }
    return ret;
  }

  ret = tlsGetAvailable(_tlsContext);
  if (ret < 0) {
    stop();
    ret = 0;
  }

  if (_peekVal >= 0) {
    ret += 1;
  }

  return ret;
}

int LTETLSClient::read()
{
  int     ret;
  uint8_t data = 0;

  ret = read(&data, 1);
  if (ret < 0) {
    return ret;
  }

  return data;
}

int LTETLSClient::read(uint8_t *buf, size_t size)
{
  ssize_t len;
  int     isPeek = 0;

  if (size && !buf) {
    LTETLSCERR("invalid parameter\n");
    return FAILED;
  }

  if (!size) {
    return 0;
  }

  len = available();
  if (len <= 0) {
    LTETLSCERR("not available\n");
    return FAILED;
  }

  if (_peekVal >= 0) {
    buf[0]   = _peekVal;
    _peekVal = -1;
    size--;
    len--;

    if (!size || !len) {
      LTETLSCDBG("read 1 byte\n");

      return 1;
    }

    buf++;
    isPeek = 1;
  }

  len = tlsRead(_tlsContext, buf, size);
  if (len < 0) {
    stop();
    return FAILED;
  }

  if (isPeek) {
    len += 1;
  }

  LTETLSCDBG("read %d byte\n", len);

  return len;
}

int LTETLSClient::peek()
{
  if (_peekVal >= 0) {
    return _peekVal;
  }

  _peekVal = timedRead();

  return _peekVal;
}

void LTETLSClient::flush()
{
  // TODO: a real check to ensure receiving has been completed
}

void LTETLSClient::stop()
{
  if (_tlsContext) {
    tlsShutdown(_tlsContext);
    delete _tlsContext;
    _tlsContext = NULL;
  }
  _connected = NOT_CONNECTED;
  _peekVal   = -1;
}

uint8_t LTETLSClient::connected()
{
  return _connected;
}

void LTETLSClient::setCACert(const char *rootCA)
{
  if (!rootCA || (strlen(rootCA) == SIZE_MAX)) {
    LTETLSCERR("invalid parameter\n");
    return;
  }

  if (_rootCA) {
    delete[] _rootCA;
    _rootCA = NULL;
  }

  _rootCASize = strlen(rootCA) + 1;
  _rootCA = new char[_rootCASize];
  if (!_rootCA) {
    LTETLSCERR("failed to allocate memory\n");
    return;
  }

  strncpy(_rootCA, rootCA, _rootCASize - 1);
  _rootCA[_rootCASize - 1] = '\0';
}

void LTETLSClient::setCACert(const unsigned char *rootCA, size_t size)
{
  if (!rootCA || (size == 0)) {
    LTETLSCERR("invalid parameter\n");
    return;
  }

  if (_rootCA) {
    delete[] _rootCA;
    _rootCA = NULL;
  }

  _rootCASize = size;
  _rootCA     = new char[_rootCASize];
  if (!_rootCA) {
    LTETLSCERR("failed to allocate memory\n");
    return;
  }

  memcpy(_rootCA, rootCA, _rootCASize);
}

void LTETLSClient::setCACert(File& f, size_t size)
{
  int ret;

  if ((size == 0) || (size == SIZE_MAX)) {
    LTETLSCERR("invalid parameter\n");
    return;
  }

  if (_rootCA) {
    delete[] _rootCA;
    _rootCA = NULL;
  }

  _rootCASize = size + 1;
  _rootCA = new char[_rootCASize];
  if (!_rootCA) {
    LTETLSCERR("failed to allocate memory\n");
    return;
  }

  ret = f.read(_rootCA, size);
  if (ret < 0 || size != static_cast<size_t>(ret)) {
    delete[] _rootCA;
    _rootCA = NULL;
    return;
  }

  _rootCA[size] = '\0';
}

void LTETLSClient::setCACert(Stream& stream, size_t size)
{
  if ((size == 0) || (size == SIZE_MAX)) {
    LTETLSCERR("invalid parameter\n");
    return;
  }

  if (_rootCA) {
    delete[] _rootCA;
    _rootCA = NULL;
  }

  _rootCASize = size + 1;
  _rootCA = new char[_rootCASize];
  if (!_rootCA) {
    LTETLSCERR("failed to allocate memory\n");
    return;
  }

  if (size != stream.readBytes(_rootCA, size)) {
    delete[] _rootCA;
    _rootCA = NULL;
    return;
  }

  _rootCA[size] = '\0';
}

void LTETLSClient::setCertificate(const char *clientCA)
{
  if (!clientCA || (strlen(clientCA) == SIZE_MAX)) {
    LTETLSCERR("invalid parameter\n");
    return;
  }

  if (_clientCA) {
    delete[] _clientCA;
    _clientCA = NULL;
  }

  _clientCASize = strlen(clientCA) + 1;
  _clientCA = new char[_clientCASize];
  if (!_clientCA) {
    LTETLSCERR("failed to allocate memory\n");
    return;
  }

  strncpy(_clientCA, clientCA, _clientCASize - 1);
  _clientCA[_clientCASize - 1] = '\0';
}

void LTETLSClient::setCertificate(const unsigned char *clientCA, size_t size)
{
  if (!clientCA || (size == 0)) {
    LTETLSCERR("invalid parameter\n");
    return;
  }

  if (_clientCA) {
    delete[] _clientCA;
    _clientCA = NULL;
  }

  _clientCASize = size;
  _clientCA = new char[_clientCASize];
  if (!_clientCA) {
    LTETLSCERR("failed to allocate memory\n");
    return;
  }

  memcpy(_clientCA, clientCA, _clientCASize);
}

void LTETLSClient::setCertificate(File& f, size_t size)
{
  int ret;

  if ((size == 0) || (size == SIZE_MAX)) {
    LTETLSCERR("invalid parameter\n");
    return;
  }

  if (_clientCA) {
    delete[] _clientCA;
    _clientCA = NULL;
  }

  _clientCASize = size + 1;
  _clientCA = new char[_clientCASize];
  if (!_clientCA) {
    LTETLSCERR("failed to allocate memory\n");
    return;
  }

  ret = f.read(_clientCA, size);
  if (ret < 0 || size != static_cast<size_t>(ret)) {
    delete[] _clientCA;
    _clientCA = NULL;
    return;
  }

  _clientCA[size] = '\0';
}

void LTETLSClient::setCertificate(Stream& stream, size_t size)
{
  if ((size == 0) || (size == SIZE_MAX)) {
    LTETLSCERR("invalid parameter\n");
    return;
  }

  if (_clientCA) {
    delete[] _clientCA;
    _clientCA = NULL;
  }

  _clientCASize = size + 1;
  _clientCA = new char[_clientCASize];
  if (!_clientCA) {
    LTETLSCERR("failed to allocate memory\n");
    return;
  }

  if (size != stream.readBytes(_clientCA, size)) {
    delete[] _clientCA;
    _clientCA = NULL;
    return;
  }

  _clientCA[size] = '\0';
}

void LTETLSClient::setPrivateKey(const char *privateKey)
{
  if (!privateKey || (strlen(privateKey) == SIZE_MAX)) {
    LTETLSCERR("invalid parameter\n");
    return;
  }

  if (_privateKey) {
    delete[] _privateKey;
    _privateKey = NULL;
  }

  _privateKeySize = strlen(privateKey) + 1;
  _privateKey = new char[_privateKeySize];
  if (!_privateKey) {
    LTETLSCERR("failed to allocate memory\n");
    return;
  }

  strncpy(_privateKey, privateKey, _privateKeySize - 1);
  _privateKey[_privateKeySize - 1] = '\0';
}

void LTETLSClient::setPrivateKey(const unsigned char *privateKey, size_t size)
{
  if (!privateKey || (size == 0)) {
    LTETLSCERR("invalid parameter\n");
    return;
  }

  if (_privateKey) {
    delete[] _privateKey;
    _privateKey = NULL;
  }

  _privateKeySize = size;
  _privateKey = new char[_privateKeySize];
  if (!_privateKey) {
    LTETLSCERR("failed to allocate memory\n");
    return;
  }

  memcpy(_privateKey, privateKey, _privateKeySize);
}

void LTETLSClient::setPrivateKey(File& f, size_t size)
{
  int ret;

  if ((size == 0) || (size == SIZE_MAX)) {
    LTETLSCERR("invalid parameter\n");
    return;
  }

  if (_privateKey) {
    delete[] _privateKey;
    _privateKey = NULL;
  }

  _privateKeySize = size + 1;
  _privateKey = new char[_privateKeySize];
  if (!_privateKey) {
    LTETLSCERR("failed to allocate memory\n");
    return;
  }

  ret = f.read(_privateKey, size);
  if (ret < 0 || size != static_cast<size_t>(ret)) {
    delete[] _privateKey;
    _privateKey = NULL;
    return;
  }

  _privateKey[size] = '\0';
}

void LTETLSClient::setPrivateKey(Stream& stream, size_t size)
{
  if ((size == 0) || (size == SIZE_MAX)) {
    LTETLSCERR("invalid parameter\n");
    return;
  }

  if (_privateKey) {
    delete[] _privateKey;
    _privateKey = NULL;
  }

  _privateKeySize = size + 1;
  _privateKey = new char[_privateKeySize];
  if (!_privateKey) {
    LTETLSCERR("failed to allocate memory\n");
    return;
  }

  if (size != stream.readBytes(_privateKey, size)) {
    delete[] _privateKey;
    _privateKey = NULL;
    return;
  }

  _privateKey[size] = '\0';
}

int LTETLSClient::setTimeout(uint32_t milliseconds)
{
  _timeout = milliseconds;

  return 0;
}

int LTETLSClient::setSendTimeout(uint32_t milliseconds)
{
  _writeTimeout = milliseconds;

  return 0;
}
