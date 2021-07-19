/*
 *  LTEUDP.cpp - LTEUDP implementation file for Spresense Arduino
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
 * @file LTEUDP.cpp
 *
 * @author Sony Semiconductor Solutions Corporation
 *
 * @brief  LTE UDP Library for Spresense Arduino.
 *
 * @details [en] By using this library, you can UDP packet to be sent and received.
 *
 * @details [ja] このライブラリを使用することで、UDPパケットを送受信できます。
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>

/* To avoid multiple define in <netinet/in.h> and <IPAddress.h> */
#ifdef INADDR_NONE
#undef INADDR_NONE
#endif

#include <IPAddress.h>

#include <LTEUDP.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef BRD_DEBUG
#define LTEUDPDBG(format, ...) ::printf("DEBUG:LTEUDP:%d " format, __LINE__, ##__VA_ARGS__)
#else
#define LTEUDPDBG(format, ...)
#endif
#define LTEUDPERR(format, ...) ::printf("ERROR:LTEUDP:%d " format, __LINE__, ##__VA_ARGS__)

#define BUFFER_MAX_LEN 1500
#define FAILED         -1
#define INVALID_FD     -1
#define BEGIN_SUCCESS  1
#define BEGIN_FAILED   0
#define END_SUCCESS    1
#define END_FAILED     0
#define PARSE_FAILED   0
#define NOT_AVAILABLE  0

/****************************************************************************
 * Public Functions
 ****************************************************************************/

LTEUDPBuffer::LTEUDPBuffer()
{
  LTEUDPBuffer(BUFFER_MAX_LEN);
}

LTEUDPBuffer::LTEUDPBuffer(size_t size): _maxSize(size), _begin(0), _end(0)
{
  if (size != 0) {
    _buf = new char[size];
    if (!_buf) {
      LTEUDPERR("failed to allocate memory\n");
    }
  }
}

LTEUDPBuffer::~LTEUDPBuffer()
{
  if (_buf) {
    delete[] _buf;
    _buf = NULL;
  }
}

size_t LTEUDPBuffer::write(uint8_t val)
{
  return write(&val, 1);
}

size_t LTEUDPBuffer::write(const uint8_t *buffer, size_t size)
{
  if ((size > _maxSize) || !_buf) {
    return 0;
  }

  memcpy(_buf, buffer, size);

  _begin = 0;
  _end   = size;

  return size;
}

int LTEUDPBuffer::available()
{
  if (!_buf) {
    return 0;
  }

  return _end - _begin;
}

int LTEUDPBuffer::read()
{
  int  size;
  char data;

  size = read(&data, 1);
  if (size > 0) {
    return data;
  }

  return FAILED;
}

int LTEUDPBuffer::read(char* buffer, size_t size)
{
  int avail_size;

  if (!_buf) {
    return FAILED;
  }

  if (!size) {
    return 0;
  }

  avail_size = available();
  if (!avail_size) {
    return FAILED;
  }

  if (size > static_cast<size_t>(avail_size)) {
    size = avail_size;
  }

  memcpy(buffer, &_buf[_begin], size);

  _begin += size;

  return size;
}

int LTEUDPBuffer::peek()
{
  if (!_buf) {
    return FAILED;
  }

  if (!available()) {
    return FAILED;
  }

  return _buf[_begin];
}


LTEUDP::LTEUDP()
: _fd(INVALID_FD)
, _wbuf(NULL)
, _wbufSize(0)
, _rbuf(NULL)
, _remotePort(0)
{
}

LTEUDP::~LTEUDP()
{
  stop();
}

uint8_t LTEUDP::begin(uint16_t port)
{
  stop();

  _wbuf = new uint8_t[BUFFER_MAX_LEN];
  if (!_wbuf) {
    LTEUDPERR("failed to allocate memory\n");
    return BEGIN_FAILED;
  }

  _fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (_fd < 0) {
    LTEUDPERR("socket() error : %d\n", errno);
    return BEGIN_FAILED;
  }

  int ret;

  struct sockaddr_in src_addr;
  memset(&src_addr, 0, sizeof(struct sockaddr_in));
  src_addr.sin_family      = AF_INET;
  src_addr.sin_port        = htons(port);
  src_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  ret = bind(_fd, reinterpret_cast<struct sockaddr*>(&src_addr),
             sizeof(struct sockaddr_in));
  if (ret < 0) {
    LTEUDPERR("bind() error : %d\n", errno);
    stop();
    return BEGIN_FAILED;
  }

  return BEGIN_SUCCESS;
}

void LTEUDP::stop()
{
  if (_wbuf) {
    delete[] _wbuf;
    _wbuf = NULL;
  }
  _wbufSize = 0;

  if (_rbuf) {
    delete _rbuf;
    _rbuf = NULL;
  }

  if (_fd != INVALID_FD) {
    close(_fd);
    _fd = INVALID_FD;
  }
}

int LTEUDP::beginPacket(IPAddress ip, uint16_t port)
{
  struct in_addr addr;
  addr.s_addr = ip;

  return beginPacket(inet_ntoa(addr), port);
}

int LTEUDP::beginPacket(const char *host, uint16_t port)
{
  int              ret;
  struct addrinfo  hints;
  struct addrinfo *ainfo = NULL;

  if (!host) {
    LTEUDPERR("invalid parameter\n");
    return BEGIN_FAILED;
  }

  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  String portStr(port);

  ret = getaddrinfo(host, portStr.c_str(), &hints, &ainfo);
  if (ret != 0) {
    LTEUDPDBG("host not found\n");
    return BEGIN_FAILED;
  }

  IPAddress ip(reinterpret_cast<struct sockaddr_in*>(ainfo->ai_addr)->sin_addr.s_addr);
  freeaddrinfo(ainfo);

  int fd = INVALID_FD;

  if (_fd == INVALID_FD) {
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
      LTEUDPERR("socket() error : %d\n", errno);
      return BEGIN_FAILED;
    }
  }

  if (!_wbuf) {
    _wbuf = new uint8_t[BUFFER_MAX_LEN];
    if (!_wbuf) {
      LTEUDPERR("failed to allocate memory\n");
      if (fd != INVALID_FD) {
        close(fd);
      }
      return BEGIN_FAILED;
    }
  }

  if (_fd == INVALID_FD) {
    _fd = fd;
  }
  _remoteIp   = ip;
  _remotePort = port;
  _wbufSize   = 0;
  ret         = BEGIN_SUCCESS;

  return ret;
}

int LTEUDP::endPacket()
{
  if (!_wbuf || (_fd == INVALID_FD)) {
    LTEUDPDBG("not available\n");
    return END_FAILED;
  }

  ssize_t            len;
  struct sockaddr_in dstaddr;

  memset(&dstaddr, 0, sizeof(dstaddr));
  dstaddr.sin_addr.s_addr = static_cast<uint32_t>(_remoteIp);
  dstaddr.sin_family      = AF_INET;
  dstaddr.sin_port        = htons(_remotePort);

  len = sendto(_fd, _wbuf, _wbufSize, 0,
               reinterpret_cast<struct sockaddr*>(&dstaddr), sizeof(dstaddr));
  if(len < 0) {
    LTEUDPERR("sendto() error : %d\n", errno);
    return END_FAILED;
  }

  LTEUDPDBG("sent %d byte\n", len);

  return END_SUCCESS;
}

size_t LTEUDP::write(uint8_t val)
{
  if (!_wbuf || (_fd == INVALID_FD)) {
    LTEUDPDBG("not available\n");
    return 0;
  }

  if (_wbufSize >= BUFFER_MAX_LEN) {
    endPacket();
    _wbufSize = 0;
  }

  _wbuf[_wbufSize++] = val;

  return 1;
}

size_t LTEUDP::write(const uint8_t *buffer, size_t size)
{
  size_t i;
  size_t ret;

  if (!buffer) {
    LTEUDPERR("invalid parameter\n");
    return 0;
  }

  for (i = 0; i < size; i++) {
    ret = write(buffer[i]);
    if (ret == 0) {
      break;
    }
  }

  return i;
}

int LTEUDP::parsePacket()
{
  struct sockaddr_in fromaddr;
  int                fromaddrlen = sizeof(fromaddr);
  int                len;

  if (_rbuf) {
    LTEUDPERR("parsePacket already\n");
    return PARSE_FAILED;
  }

  if (_fd == INVALID_FD) {
    LTEUDPDBG("not available\n");
    return PARSE_FAILED;
  }

  char *buf = new char[BUFFER_MAX_LEN];
  if (!buf) {
    LTEUDPERR("failed to allocate memory\n");
    return PARSE_FAILED;
  }

  len = recvfrom(_fd, buf, BUFFER_MAX_LEN, MSG_DONTWAIT,
                 reinterpret_cast<struct sockaddr*>(&fromaddr),
                 reinterpret_cast<socklen_t*>(&fromaddrlen));
  if (len < 0) {
    if (errno != EAGAIN) {
      LTEUDPERR("recvfrom() error : %d\n", errno);
    } else {
      usleep(10);
    }
    delete[] buf;
    return PARSE_FAILED;
  }

  _remoteIp   = IPAddress(fromaddr.sin_addr.s_addr);
  _remotePort = ntohs(fromaddr.sin_port);

  if (len > 0) {
    _rbuf = new LTEUDPBuffer(BUFFER_MAX_LEN);
    if (!_rbuf) {
      LTEUDPERR("failed to allocate memory\n");
      delete[] buf;
      return PARSE_FAILED;
    }
    _rbuf->write(reinterpret_cast<const uint8_t*>(buf), len);
  }
  delete[] buf;

  LTEUDPDBG("received %d byte\n", len);

  return len;
}

int LTEUDP::available()
{
  if (!_rbuf) {
    LTEUDPDBG("not available\n");
    return NOT_AVAILABLE;
  }

  return _rbuf->available();
}

int LTEUDP::read()
{
  int val;
  int remain_len;

  if (!_rbuf) {
    LTEUDPDBG("not available\n");
    return FAILED;
  }

  val = _rbuf->read();

  remain_len = _rbuf->available();
  if (remain_len == 0) {
    delete _rbuf;
    _rbuf = NULL;
  }

  return val;
}

int LTEUDP::read(unsigned char* buffer, size_t len)
{
  return read((char*)buffer, len);
}

int LTEUDP::read(char* buffer, size_t len)
{
  int read_len;
  int remain_len;

  if (!buffer) {
    LTEUDPERR("invalid parameter\n");
    return FAILED;
  }

  if (!_rbuf) {
    LTEUDPDBG("not available\n");
    return FAILED;
  }

  read_len = _rbuf->read(buffer, len);

  remain_len = _rbuf->available();
  if (remain_len == 0) {
    delete _rbuf;
    _rbuf = NULL;
  }

  return read_len;
}

int LTEUDP::peek()
{
  if (!_rbuf) {
    LTEUDPDBG("not available\n");
    return FAILED;
  }

  return _rbuf->peek();
}

void LTEUDP::flush()
{
  if (_rbuf) {
    delete _rbuf;
    _rbuf = NULL;
  }
}

IPAddress LTEUDP::remoteIP()
{
  return _remoteIp;
}

uint16_t LTEUDP::remotePort()
{
  return _remotePort;
}

int LTEUDP::setTimeout(uint32_t milliseconds)
{
  int ret;
  struct timeval tv;

  tv.tv_sec = milliseconds / 1000;
  tv.tv_usec = (milliseconds - (tv.tv_sec * 1000)) * 1000;
  ret = setsockopt(_fd, SOL_SOCKET, SO_RCVTIMEO,
                   reinterpret_cast<const void*>(&tv),
                   sizeof(struct timeval));
  if (ret < 0) {
    LTEUDPERR("setsockopt(SO_RCVTIMEO) error : %d\n", errno);
  } else {
    ret = setsockopt(_fd, SOL_SOCKET, SO_SNDTIMEO,
                     reinterpret_cast<const void*>(&tv),
                     sizeof(struct timeval));
    if (ret < 0) {
      LTEUDPERR("setsockopt(SO_SNDTIMEO) error : %d\n", errno);
    }
  }

  return ret;
}
