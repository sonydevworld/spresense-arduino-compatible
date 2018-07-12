/*
  HardwareSerial.cpp - Serial port I/O for the Sparduino SDK
  Copyright (C) 2018 Sony Semiconductor Solutions Corp.
  Copyright (c) 2017 Sony Corporation  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <sdk/config.h>

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <nuttx/fs/ioctl.h>
#include <nuttx/serial/tioctl.h>
#include <HardwareSerial.h>

HardwareSerial::HardwareSerial(uint8_t ch)
: _fd(-1),
 _ch(ch),
 _peek_buffer(-1),
 _available(0),
 _wbuf_size(0),
 _stdin_fd(dup(0))
{
}

void HardwareSerial::begin(unsigned long baud, uint8_t config)
{
    int ret;
    struct termios tio;
    const char* dev = 0;
    char node[8];
    uint8_t tty;

    if (_fd >= 0) {
        ::close(_fd);
        _fd = -1;
    }

    if (config != SERIAL_8N1) {
        printf("UART only supports SERIAL_8N1\n");
        return;
    }

    if ((ret = ch_to_tty(&tty)))
        return;

    sprintf(node, "/dev/ttyS%1u", tty);
    dev = node;

#if defined(CONFIG_UART1_SERIAL_CONSOLE) || defined(CONFIG_UART2_SERIAL_CONSOLE)
#ifdef CONFIG_UART1_SERIAL_CONSOLE
    if (_ch == 1)
#elif CONFIG_UART2_SERIAL_CONSOLE
    if (_ch == 2)
#endif
    {
        dev = "/dev/console";
        // Redirect /dev/null to stdin
        int null = open("/dev/null", O_RDONLY);

        close(0);
        if (null >= 0 && null != 0) {
            fs_dupfd2(null, 0);
            close(null);
        }
    }
#endif

    _fd = open(dev, O_RDWR);
    if (_fd < 0)
        return;

    // Apply baud rate
    ret = ioctl(_fd, TCGETS, (long unsigned int)&tio);
    if (ret != 0)
        return;
    tio.c_speed = baud;
    tio.c_cflag = config;
    tio.c_oflag &= ~OPOST;
    ioctl(_fd, TCSETS, (long unsigned int)&tio);
    ioctl(_fd, TCFLSH, NULL);

    _wbuf_size = availableForWrite();
}

void HardwareSerial::end(void)
{
    if (_fd >= 0) {
        close(_fd);
        _fd = -1;
    }
    fs_dupfd2(_stdin_fd, 0);
}

HardwareSerial::operator bool() const
{
    return (_fd >= 0);
}

int HardwareSerial::available(void)
{
    int ret;
    int count = 0;

    if (_fd < 0)
        return 0;

    ret = ioctl(_fd, FIONREAD, (long unsigned int)&count);
    if (ret)
        printf("Serial FIONREAD not supported\n");

    return count;
}

int HardwareSerial::peek(void)
{
    char buf[1];

    if (_fd < 0)
        return -1;

    if (_peek_buffer > 0)
        return _peek_buffer;

    if (_available <= 0) {
        _available = available();
        if (_available <= 0)
            return -1;
    }

    ::read(_fd, buf, 1);
    --_available;
    _peek_buffer = (unsigned int)buf[0];

    return _peek_buffer;
}

int HardwareSerial::read(void)
{
    char buf[1];

    if (_fd < 0)
        return -1;

    if (_peek_buffer >= 0) {
		int t = _peek_buffer;
		_peek_buffer = -1;
		return t;
	}

    if (_available <= 0)
        _available = available();

    if (_available <= 0)
        return -1;

    ::read(_fd, buf, 1);
    --_available;

    return (unsigned int)buf[0];
}

int HardwareSerial::availableForWrite(void)
{
    int ret;
    int count = 0;

    if (_fd < 0)
        return 0;

    ret = ioctl(_fd, FIONSPACE, (long unsigned int)&count);
    if (ret)
        printf("Serial FIONSPACE not supported\n");

    return count;
}

void HardwareSerial::flush(void)
{
    ioctl(_fd, TCFLSH, NULL);
    while (availableForWrite() != _wbuf_size)
        usleep(1000);
}

size_t HardwareSerial::write(const char* str)
{
    if (_fd < 0)
        return 0;

    return ::write(_fd, str, strlen((const char*)str));
}

size_t HardwareSerial::write(uint8_t c)
{
    return write(&c, 1);
}

size_t HardwareSerial::write(const uint8_t* buffer, size_t size)
{
    if (_fd < 0)
        return 0;

    return ::write(_fd, buffer, size);
}

#define UART_CH_NUM 3
#define UART_0 0
#define UART_1 1
#define UART_2 2
#define TTYS_0 0
#define TTYS_1 1
#define TTYS_2 2
int HardwareSerial::ch_to_tty(uint8_t *tty)
{
    /* please refer Spresense SDK cxd56_serial.c */
    int ttys[UART_CH_NUM];
    uint8_t ch;

    if (_ch >= UART_CH_NUM) {
        printf("invalid channel.\n");
        return -1;
    }

    for (ch = 0; ch < UART_CH_NUM; ch++)
        ttys[ch] = -1;

#if defined(CONFIG_UART0_SERIAL_CONSOLE) || defined(CONFIG_UART1_SERIAL_CONSOLE) || defined(CONFIG_UART2_SERIAL_CONSOLE)
#define HAVE_CONSOLE 1
#endif

#ifdef HAVE_CONSOLE
#  if defined(CONFIG_UART0_SERIAL_CONSOLE)
    ttys[UART_0] = TTYS_0;
#    ifdef CONFIG_CXD56_UART1
    ttys[UART_1] = TTYS_1;
#      ifdef CONFIG_CXD56_UART2
    ttys[UART_2] = TTYS_2;
#      endif
#    else
#      ifdef CONFIG_CXD56_UART2
    ttys[UART_2] = TTYS_1;
#      endif
#    endif
#  elif defined(CONFIG_UART1_SERIAL_CONSOLE)
    ttys[UART_1] = TTYS_0;
#    ifdef CONFIG_CXD56_UART0
    ttys[UART_0] = TTYS_1;
#      ifdef CONFIG_CXD56_UART2
    ttys[UART_2] = TTYS_2;
#      endif
#    else
#      ifdef CONFIG_CXD56_UART2
    ttys[UART_2] = TTYS_1;
#      else
    /* do nothing */
#      endif
#    endif
#  elif defined(CONFIG_UART2_SERIAL_CONSOLE)
    ttys[UART_2] = TTYS_0;
#    ifdef CONFIG_CXD56_UART0
    ttys[UART_0] = TTYS_1;
#      ifdef CONFIG_CXD56_UART1
    ttys[UART_1] = TTYS_2;
#      endif
#    else
#      ifdef CONFIG_CXD56_UART1
    ttys[UART_1] = TTYS_1;
#      endif
#    endif
#  endif
#else
    ttys[UART_0] = TTYS_0;
#  ifdef CONFIG_CXD56_UART1
    ttys[UART_1] = TTYS_1;
#    ifdef CONFIG_CXD56_UART2
    ttys[UART_2] = TTYS_2;
#    endif
#  else
#    ifdef CONFIG_CXD56_UARTUART_2
    ttys[UART_2] = TTYS_1;
#    else
    /* do nothing */
#    endif
#  endif
#endif

	if (ttys[_ch] < 0) {
		printf("invalid channel.\n");
		return -1;
	}

	*tty = (uint8_t)ttys[_ch];
	return 0;
}

#if defined(CONFIG_CXD56_UART1)
HardwareSerial Serial1(1);
#endif

#if defined(CONFIG_CXD56_UART2)
HardwareSerial Serial2(2);
#endif

#if defined(CONFIG_CXD56_UART1)
HardwareSerial &Serial = Serial1;
#elif defined(CONFIG_CXD56_UART2)
HardwareSerial &Serial = Serial2;
#endif

extern "C" {

void serialEvent(void) __attribute__((weak));
void serialEventRun(void) __attribute__((weak));

void serialEventRun(void)
{
    if (serialEvent && (Serial.available() > 0))
        serialEvent();
}

}
