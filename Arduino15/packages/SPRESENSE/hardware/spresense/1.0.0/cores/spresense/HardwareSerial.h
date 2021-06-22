/*
  HardwareSerial.h - Serial port I/O for the Spresense SDK
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
#include <termios.h>
#include "Stream.h"


#define SERIAL_5N1 CS5
#define SERIAL_6N1 CS6
#define SERIAL_7N1 CS7
#define SERIAL_8N1 CS8
#define SERIAL_5N2 (CS5 | CSTOPB)
#define SERIAL_6N2 (CS6 | CSTOPB)
#define SERIAL_7N2 (CS7 | CSTOPB)
#define SERIAL_8N2 (CS8 | CSTOPB)
#define SERIAL_5E1 (CS5 | PARENB)
#define SERIAL_6E1 (CS6 | PARENB)
#define SERIAL_7E1 (CS7 | PARENB)
#define SERIAL_8E1 (CS8 | PARENB)
#define SERIAL_5E2 (CS5 | CSTOPB | PARENB)
#define SERIAL_6E2 (CS6 | CSTOPB | PARENB)
#define SERIAL_7E2 (CS7 | CSTOPB | PARENB)
#define SERIAL_8E2 (CS8 | CSTOPB | PARENB)
#define SERIAL_5O1 (CS5 | PARENB | PARODD)
#define SERIAL_6O1 (CS6 | PARENB | PARODD)
#define SERIAL_7O1 (CS7 | PARENB | PARODD)
#define SERIAL_8O1 (CS8 | PARENB | PARODD)
#define SERIAL_5O2 (CS5 | CSTOPB | PARENB | PARODD)
#define SERIAL_6O2 (CS6 | CSTOPB | PARENB | PARODD)
#define SERIAL_7O2 (CS7 | CSTOPB | PARENB | PARODD)
#define SERIAL_8O2 (CS8 | CSTOPB | PARENB | PARODD)

#define SERIAL_CTS CCTS_OFLOW
#define SERIAL_RTS CRTS_IFLOW
#define SERIAL_RTSCTS CRTSCTS

#define SERIAL_CONTROL_MASK (CSIZE | \
                             CSTOPB | \
                             CREAD | \
                             PARENB | \
                             PARODD | \
                             HUPCL | \
                             CLOCAL | \
                             CCTS_OFLOW | \
                             CRTS_IFLOW)

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
    void begin(unsigned long baud, uint16_t config);
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
