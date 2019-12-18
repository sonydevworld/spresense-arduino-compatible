/*
 *  LTEClient.cpp - LTEClient implementation file for Spresense Arduino
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
 * @file LTEClient.cpp
 *
 * @author Sony Semiconductor Solutions Corporation
 *
 * @brief LTE Client Library for Spresense Arduino.
 *
 * @details [en] By using this library, you can connect to servers and send and receive data.
 *
 * @details [ja] このライブラリを使用することで、サーバーに接続してデータを送受信できます。
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

/* To avoid multiple define in <netinet/in.h> and <IPAddress.h> */
#ifdef INADDR_NONE
#undef INADDR_NONE
#endif

#include <IPAddress.h>

#include <LTEClient.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef BRD_DEBUG
#define LTECDBG(format, ...) printf("DEBUG:LTEClient:%d " format, __LINE__, ##__VA_ARGS__)
#else
#define LTECDBG(format, ...)
#endif
#define LTECERR(format, ...) printf("ERROR:LTEClient:%d " format, __LINE__, ##__VA_ARGS__)

#define BUFFER_MAX_LEN 1500
#define INVALID_FD     -1
#define NOT_CONNECTED  0
#define CONNECTED      1
#define NOT_AVAILABLE  0
#define FAILED         -1

/****************************************************************************
 * Public Functions
 ****************************************************************************/

LTEClient::LTEClient(): _fd(INVALID_FD), _buf(NULL), _connected(NOT_CONNECTED)
{
}

LTEClient::~LTEClient()
{
  stop();
}

int LTEClient::connect(IPAddress ip, uint16_t port)
{
  struct in_addr addr;
  addr.s_addr = ip;

  return connect(inet_ntoa(addr), port);
}

int LTEClient::connect(const char *host, uint16_t port)
{
  int              ret;
  struct addrinfo  hints;
  struct addrinfo *ainfo     = NULL;
  struct addrinfo *curainfo = NULL;

  if (!host) {
    LTECERR("invalid parameter\n");
    return NOT_CONNECTED;
  }

  stop();

  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  String portStr(port);

  ret = getaddrinfo(host, portStr.c_str(), &hints, &ainfo);
  if (ret != 0) {
    LTECERR("getaddrinfo() error : %d\n", ret);
    return NOT_CONNECTED;
  }

  for (curainfo = ainfo; curainfo != NULL; curainfo = curainfo->ai_next) {
    _fd = socket(curainfo->ai_family, curainfo->ai_socktype,
                 curainfo->ai_protocol);
    if (_fd < 0) {
      LTECERR("socket() error : %d\n", errno);
      break;
    }

    ret = ::connect(_fd, curainfo->ai_addr, curainfo->ai_addrlen);
    if (ret < 0) {
      LTECERR("connect() error : %d\n", errno);
      close(_fd);
      _fd = INVALID_FD;
    } else {
      /* connect succeeded */
      break;
    }
  }
  freeaddrinfo(ainfo);

  if (_fd == INVALID_FD) {
    return NOT_CONNECTED;
  }

  fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL) | O_NONBLOCK);

  _buf = new uint8_t[BUFFER_MAX_LEN];
  if (!_buf) {
    LTECERR("failed to allocate memory\n");
    close(_fd);
    _fd = INVALID_FD;
    return NOT_CONNECTED;
  }
  _connected = CONNECTED;

  LTECDBG("connected to %s\n", host);

  return CONNECTED;
}

size_t LTEClient::write(uint8_t val)
{
  return write(&val, 1);
}

size_t LTEClient::write(const uint8_t *buf, size_t size)
{
  int      ret;
  uint8_t *buf_ptr    = const_cast<uint8_t*>(buf);
  size_t   remain_len = size;

  if (!size || !buf) {
    LTECERR("invalid parameter\n");
    return 0;
  }

  if (!_connected) {
    LTECDBG("not connected\n");
    return 0;
  }

  do {
    ret = send(_fd, buf_ptr, remain_len, 0);
    if (ret == 0) {
      break;
    } else if (ret < 0) {
      LTECERR("send() error : %d\n", errno);
      break;
    }

    remain_len -= ret;
    buf_ptr    += ret;
  } while (0 < remain_len);

  LTECDBG("written %d byte\n", size - remain_len);

  return size - remain_len;
}

int LTEClient::available()
{
  ssize_t len;

  if (!_buf) {
    LTECDBG("not available\n");
    return NOT_AVAILABLE;
  }

  len = recv(_fd, _buf, BUFFER_MAX_LEN, MSG_PEEK);
  if (len < 0) {
    if (errno != EAGAIN) {
      LTECERR("recv() error : %d\n", errno);
      stop();
    } else {
      len = NOT_AVAILABLE;
    }
  } else if (len == 0) {
    /* 0 means disconnected from server */
    stop();
  }

  return len;
}

int LTEClient::read()
{
  int     ret;
  uint8_t data = 0;

  ret = read(&data, 1);
  if (ret < 0) {
    return ret;
  }

  return data;
}

int LTEClient::read(uint8_t *buf, size_t size)
{
  ssize_t len;

  if (size && !buf) {
    LTECERR("invalid parameter\n");
    return FAILED;
  }

  if (!_buf) {
    LTECDBG("not available\n");
    return FAILED;
  }

  if (!size) {
    return 0;
  }

  len = recv(_fd, buf, size, 0);
  if (len < 0) {
    if (errno != EAGAIN) {
      LTECERR("recv() error : %d\n", errno);
      stop();
      len = FAILED;
    }
  } else if (len == 0) {
    /* 0 means disconnected from server */
    stop();
    len = FAILED;
  }

  LTECDBG("read %d byte\n", len);

  return len;
}

int LTEClient::peek()
{
  ssize_t len;

  if (!_buf) {
    LTECDBG("not available\n");
    return FAILED;
  }

  len = recv(_fd, _buf, 1, MSG_PEEK);
  if (len < 0) {
    if (errno != EAGAIN) {
      LTECERR("recv() error : %d\n", errno);
      stop();
    }
    return FAILED;
  } else if (len == 0) {
    /* 0 means disconnected from server */
    stop();
    return FAILED;
  }

  return _buf[0];
}

void LTEClient::flush()
{
  // TODO: a real check to ensure receiving has been completed
}

void LTEClient::stop()
{
  if (_buf) {
    delete[] _buf;
    _buf = NULL;
  }
  _connected = NOT_CONNECTED;
  if (_fd != INVALID_FD) {
    close(_fd);
    _fd = INVALID_FD;
  }
}

uint8_t LTEClient::connected()
{
  ssize_t len;

  if (_connected) {
    len = recv(_fd, _buf, 0, 0);
    if (len < 0) {
      if (errno != EAGAIN) {
        LTECERR("recv() error : %d\n", errno);
        stop();
      }
    }
  }

  return _connected;
}

