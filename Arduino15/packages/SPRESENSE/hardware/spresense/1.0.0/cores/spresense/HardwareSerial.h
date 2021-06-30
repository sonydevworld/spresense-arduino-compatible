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
#include "Stream.h"

/* Bit defintions (like c_cflag in the termios structure) */

#define MY_CSIZE     (3 << 4)  /* Bits 4-5: Character size: */
#  define MY_CS5     (0 << 4)  /*   5 bits */
#  define MY_CS6     (1 << 4)  /*   6 bits */
#  define MY_CS7     (2 << 4)  /*   7 bits */
#  define MY_CS8     (3 << 4)  /*   8 bits */
#define MY_CSTOPB    (1 << 6)  /* Bit 6: Send two stop bits, else one */
#define MY_PARENB    (1 << 8)  /* Bit 8: Parity enable */
#define MY_PARODD    (1 << 9)  /* Bit 9: Odd parity, else even */

#define SERIAL_5N1 MY_CS5
#define SERIAL_6N1 MY_CS6
#define SERIAL_7N1 MY_CS7
#define SERIAL_8N1 MY_CS8
#define SERIAL_5N2 (MY_CS5 | MY_CSTOPB)
#define SERIAL_6N2 (MY_CS6 | MY_CSTOPB)
#define SERIAL_7N2 (MY_CS7 | MY_CSTOPB)
#define SERIAL_8N2 (MY_CS8 | MY_CSTOPB)
#define SERIAL_5E1 (MY_CS5 | MY_PARENB)
#define SERIAL_6E1 (MY_CS6 | MY_PARENB)
#define SERIAL_7E1 (MY_CS7 | MY_PARENB)
#define SERIAL_8E1 (MY_CS8 | MY_PARENB)
#define SERIAL_5E2 (MY_CS5 | MY_CSTOPB | MY_PARENB)
#define SERIAL_6E2 (MY_CS6 | MY_CSTOPB | MY_PARENB)
#define SERIAL_7E2 (MY_CS7 | MY_CSTOPB | MY_PARENB)
#define SERIAL_8E2 (MY_CS8 | MY_CSTOPB | MY_PARENB)
#define SERIAL_5O1 (MY_CS5 | MY_PARENB | MY_PARODD)
#define SERIAL_6O1 (MY_CS6 | MY_PARENB | MY_PARODD)
#define SERIAL_7O1 (MY_CS7 | MY_PARENB | MY_PARODD)
#define SERIAL_8O1 (MY_CS8 | MY_PARENB | MY_PARODD)
#define SERIAL_5O2 (MY_CS5 | MY_CSTOPB | MY_PARENB | MY_PARODD)
#define SERIAL_6O2 (MY_CS6 | MY_CSTOPB | MY_PARENB | MY_PARODD)
#define SERIAL_7O2 (MY_CS7 | MY_CSTOPB | MY_PARENB | MY_PARODD)
#define SERIAL_8O2 (MY_CS8 | MY_CSTOPB | MY_PARENB | MY_PARODD)

#define SERIAL_CTS (0x1000)
#define SERIAL_RTS (0x2000)
#define SERIAL_RTSCTS (SERIAL_CTS | SERIAL_RTS)

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
