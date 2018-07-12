/*
  HardwareSerial.h - Serial port I/O for the Sparduino SDK
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
#ifndef HardwareSerial_h
#define HardwareSerial_h

/*
  This header file maybe inclued in plain C file.
  To avoid compiling error all C++ stuff should be ignored
 */
#ifdef __cplusplus

#include <nuttx/config.h>
#include <sdk/config.h>
#include "Stream.h"


#define SERIAL_5N1 0x00
#define SERIAL_6N1 0x02
#define SERIAL_7N1 0x04
#define SERIAL_8N1 0x06
#define SERIAL_5N2 0x08
#define SERIAL_6N2 0x0A
#define SERIAL_7N2 0x0C
#define SERIAL_8N2 0x0E
#define SERIAL_5E1 0x20
#define SERIAL_6E1 0x22
#define SERIAL_7E1 0x24
#define SERIAL_8E1 0x26
#define SERIAL_5E2 0x28
#define SERIAL_6E2 0x2A
#define SERIAL_7E2 0x2C
#define SERIAL_8E2 0x2E
#define SERIAL_5O1 0x30
#define SERIAL_6O1 0x32
#define SERIAL_7O1 0x34
#define SERIAL_8O1 0x36
#define SERIAL_5O2 0x38
#define SERIAL_6O2 0x3A
#define SERIAL_7O2 0x3C
#define SERIAL_8O2 0x3E


#if defined(CONFIG_CXD56_UART1)
#define SERIAL_DEFAULT_CHANNEL 1
#elif defined(CONFIG_CXD56_UART2)
#define SERIAL_DEFAULT_CHANNEL 2
#elif defined(CONFIG_UART1_SERIAL_CONSOLE) || defined(CONFIG_UART2_SERIAL_CONSOLE)
#error Please enable UART in NuttX
#else
/* in this case, no objects will be created */
#define SERIAL_DEFAULT_CHANNEL 0
#endif

class HardwareSerial : public Stream
{
  public:
    HardwareSerial(uint8_t channel = SERIAL_DEFAULT_CHANNEL);
    void begin(unsigned long baud) { begin(baud, SERIAL_8N1); }
    void begin(unsigned long baud, uint8_t config);
    void end(void);

    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    int availableForWrite(void);
    virtual void flush(void);
    virtual size_t write(uint8_t c);
    virtual size_t write(const uint8_t* buffer, size_t size);
    virtual size_t write(const char* str);
    operator bool() const;

  private:
      int _fd;
      int _ch;
      int _peek_buffer;
      int _available;
      int _wbuf_size;
      int _stdin_fd;

  private:
    int ch_to_tty(uint8_t *tty);
};

#if defined(CONFIG_CXD56_UART1)
extern HardwareSerial Serial1;
#endif

#if defined(CONFIG_CXD56_UART2)
extern HardwareSerial Serial2;
#endif

#if defined(CONFIG_CXD56_UART1) || defined(CONFIG_CXD56_UART2)
extern HardwareSerial &Serial;
#endif

extern "C" void serialEvent(void);

#endif //__cplusplus
#endif //HardwareSerial_h
