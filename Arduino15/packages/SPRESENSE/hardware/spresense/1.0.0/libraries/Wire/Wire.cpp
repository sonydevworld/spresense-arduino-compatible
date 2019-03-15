/*
  Wire.cpp - Two Wire I/O for the Spresense SDK
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

#include <string.h>
#include <stdio.h>
extern "C" {
#include <cxd56_i2c.h>
#include <cxd56_pmic.h>
}
#include <errno.h>
#include "utility.h"
#include "Wire.h"

#define WIRE_PORT (0) // I2C0

TwoWire::TwoWire()
: _dev(0),
  _freq(TWI_FREQ_100KHZ),
  _transmitting(false),
  _tx_address(0),
  _tx_addr_len(TWI_ADDR_LEN_7_BIT),
  _tx_buf_index(0),
  _tx_buf_len(0),
  _rx_buf_index(0),
  _rx_buf_len(0),
  _on_receive(0),
  _on_request(0)
{
    memset(_tx_buf, 0, sizeof(_tx_buf));
    memset(_rx_buf, 0, sizeof(_rx_buf));
}

void TwoWire::begin()
{
    if (_dev == 0)
        _dev = cxd56_i2cbus_initialize(WIRE_PORT);

    if (_dev == 0)
        printf("ERROR: Failed to init I2C device\n");
}

void TwoWire::begin(uint8_t address)
{
    unuse(address);
    printf("ERROR: I2C slave mode not supported on CXD5602\n");
}

void TwoWire::begin(uint16_t address)
{
    unuse(address);
    printf("ERROR: I2C slave mode not supported on CXD5602\n");
}

void TwoWire::end()
{
    if (_dev) {
        (void) cxd56_i2cbus_uninitialize(_dev);
        _dev = 0;
    }
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop)
{
    struct i2c_msg_s msg;
    unsigned int flags = 0;
    int ret;

    if (!_dev) return 0;

    // clamp to buffer length
    if (quantity > TWI_RX_BUF_LEN)
        quantity = TWI_RX_BUF_LEN;

    if (_tx_addr_len == TWI_ADDR_LEN_10_BIT) {
        flags |= I2C_M_TEN;
    }

    if (!sendStop) {
        flags |= I2C_M_NOSTOP;
    }

    // Setup for the transfer
    msg.frequency = _freq;
    msg.addr      = address;
    msg.flags     = (flags | I2C_M_READ);
    msg.buffer    = _rx_buf;
    msg.length    = quantity;

    // Then perform the transfer.
    ret = I2C_TRANSFER(_dev, &msg, 1);
    if (ret < 0) {
        printf("ERROR: Failed to read from i2c (errno = %d)\n", errno);
        return 0;
    }

    // set rx buffer iterator vars
    _rx_buf_index = 0;
    _rx_buf_len = quantity;

    return quantity;
}

void TwoWire::beginTransmission(uint16_t address, uint8_t length)
{
    // indicate that we are transmitting
    _transmitting = true;
    // set address of targeted slave
    if (length == TWI_ADDR_LEN_7_BIT) {
        _tx_addr_len = TWI_ADDR_LEN_7_BIT;
        _tx_address = address; // cxd56_i2c.c::cxd56_i2c_transfer only takes the low 7-bit of address
                               // so I2C_WRITEADDR8(address) can't be used here
    }
    else {
        // cxd56_i2c.c::cxd56_i2c_transfer does not support 10-bit address currently
        uint8_t ten_high = I2C_WRITEADDR10H(address);
        uint8_t ten_low = I2C_WRITEADDR10L(address);
        _tx_address = (ten_high << 8) | ten_low;
        _tx_addr_len = TWI_ADDR_LEN_10_BIT;
    }
    // reset tx buffer iterator vars
    _tx_buf_index = 0;
    _tx_buf_len = 0;
}

uint8_t TwoWire::endTransmission(bool sendStop)
{
    struct i2c_msg_s msg;
    unsigned int flags = 0;
    int ret;

    if (!_dev || !_transmitting) return TWI_OTHER_ERROR;

    if (_tx_addr_len == TWI_ADDR_LEN_10_BIT) {
        flags |= I2C_M_TEN;
    }

    if (!sendStop) {
        flags |= I2C_M_NOSTOP;
    }

    // Setup for the transfer
    msg.frequency = _freq;
    msg.addr      = _tx_address;
    msg.flags     =  flags;
    msg.buffer    = _tx_buf;
    msg.length    = _tx_buf_len;

    // Then perform the transfer.
    ret = I2C_TRANSFER(_dev, &msg, 1);
    // reset tx buffer iterator vars
    _tx_buf_index = 0;
    _tx_buf_len = 0;
    // indicate that we are done transmitting
    _transmitting = false;

    if (ret == -ENODEV) {
        // device not found
        return TWI_NACK_ON_ADDRESS;
    }
    return (ret < 0) ? TWI_OTHER_ERROR : TWI_SUCCESS;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::available()
{
    return _rx_buf_len - _rx_buf_index;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::peek()
{
    if (_rx_buf_index < _rx_buf_len)
        return _rx_buf[_rx_buf_index];
    return -1;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::read()
{
    int value = -1;

    // get each successive byte on each call
    if (_rx_buf_index < _rx_buf_len) {
        value = _rx_buf[_rx_buf_index];
        ++_rx_buf_index;
    }

    return value;
}

void TwoWire::flush()
{
    // nothing to do
    return;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(uint8_t value)
{
    if (_transmitting) {
        // in master transmitter mode
        // don't bother if buffer is full
        if (_tx_buf_len >= TWI_TX_BUF_LEN) {
            setWriteError();
            return 0;
        }
        // put byte in tx buffer
        _tx_buf[_tx_buf_index] = value;
        ++_tx_buf_index;
        // update amount in buffer
        _tx_buf_len = _tx_buf_index;
    }
    else {
        // in slave send mode
        // reply to master
        printf("ERROR: I2C slave mode not supported on CXD5602\n");
    }
    return 1;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(const uint8_t* data, size_t quantity)
{
    if (_transmitting) {
        // in master transmitter mode
        for (size_t i = 0; i < quantity; ++i)
            write(data[i]);
    }
    else {
        // in slave send mode
        // reply to master
        printf("ERROR: I2C slave mode not supported on CXD5602\n");
    }
    return quantity;
}

void TwoWire::setClock(uint32_t clock)
{
    if (_freq != clock) {
        _freq = clock;
    }
}

// sets function called on slave write
void TwoWire::onReceive(TWIReceiveHandler handler)
{
    _on_receive = handler;
}

// sets function called on slave read
void TwoWire::onRequest(TWIRequestHandler handler)
{
    _on_request = handler;
}

#ifdef CONFIG_CXD56_I2C0
TwoWire Wire;
#endif
