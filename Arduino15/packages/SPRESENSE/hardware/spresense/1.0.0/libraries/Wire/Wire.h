/*
  Wire.h - Two Wire I/O for the Sparduino SDK
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
#ifndef Wire_h
#define Wire_h

/*
  This header file maybe inclued in plain C file.
  To avoid compiling error all C++ stuff should be ignored
 */
#ifdef __cplusplus

#include <stdbool.h>
#include <nuttx/config.h>
#include <nuttx/i2c/i2c_master.h>
#include <sdk/config.h>
#include "Stream.h"

// I2C frequence supported
#define TWI_FREQ_100KHZ     (100000)    // standard mode
#define TWI_FREQ_400KHZ     (400000)    // fast mode
#define TWI_FREQ_1MHZ       (1000000)   // fast mode plus

// I2C address length supported
#define TWI_ADDR_LEN_7_BIT  (7)
#define TWI_ADDR_LEN_10_BIT (10)

// buffer
#define BUFFER_LENGTH       (32)
#define TWI_TX_BUF_LEN      BUFFER_LENGTH
#define TWI_RX_BUF_LEN      BUFFER_LENGTH

// WIRE_HAS_END means Wire has end()
#define WIRE_HAS_END        (1)

// return value
#define TWI_SUCCESS         (0) // success
#define TWI_DATA_TOO_LONG   (1) // data too long to fit in transmit buffer
#define TWI_NACK_ON_ADDRESS (2) // received NACK on transmit of address
#define TWI_NACK_ON_DATA    (3) // received NACK on transmit of data
#define TWI_OTHER_ERROR     (4) // other error

typedef void (*TWIReceiveHandler)(int bytes);
typedef void (*TWIRequestHandler)(void);

class TwoWire : public Stream
{
public:
    TwoWire(void);
    void begin(void); // master mode
    void begin(uint8_t address); // slave mode, not supported in nuttx?
    void begin(uint16_t address);
    void begin(int address) { begin((uint8_t)address); }
    void end(void);

    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);
    virtual size_t write(uint8_t value);
    virtual size_t write(const uint8_t* data, size_t length);

    uint8_t requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop);
    uint8_t requestFrom(uint8_t address, uint8_t quantity) { return requestFrom(address, quantity, (uint8_t)true); }
    uint8_t requestFrom(int address, int quantity, int sendStop) { return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)sendStop); }
    uint8_t requestFrom(int address, int quantity) { return requestFrom((uint8_t)address, (uint8_t)quantity); }

    void beginTransmission(uint8_t address) { beginTransmission(address, TWI_ADDR_LEN_7_BIT); }
    void beginTransmission(uint16_t address) { beginTransmission(address, TWI_ADDR_LEN_10_BIT); }
    void beginTransmission(int address) { beginTransmission((uint8_t)address); }
    uint8_t endTransmission(bool sendStop);
    uint8_t endTransmission(void) { return endTransmission(true); }

    void setClock(uint32_t clock);
    void onReceive(TWIReceiveHandler handler);
    void onRequest(TWIRequestHandler handler);
    using Print::write;

private:
    void beginTransmission(uint16_t address, uint8_t length);

private:
    FAR struct i2c_master_s* _dev;
    uint32_t _freq;
    bool _transmitting;
    uint16_t _tx_address;
    uint8_t _tx_addr_len;
    uint8_t _tx_buf[TWI_TX_BUF_LEN];
    uint8_t _tx_buf_index;
    uint8_t _tx_buf_len;
    uint8_t _rx_buf[TWI_RX_BUF_LEN];
    uint8_t _rx_buf_index;
    uint8_t _rx_buf_len;
    TWIReceiveHandler _on_receive;
    TWIRequestHandler _on_request;
};

#ifdef CONFIG_CXD56_I2C0
extern TwoWire Wire;
#else
#error Please enable I2C0 in NuttX
#endif

#endif //__cplusplus
#endif //Wire_h
