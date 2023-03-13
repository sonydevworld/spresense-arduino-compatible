/*
 *  SoftwareSerial.cpp - Spresense Arduino Software Serial library
 *  Copyright 2018 Sony Semiconductor Solutions Corporation
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
 * @file SoftwareSerial.cpp
 * @author Sony Semiconductor Solutions Corporation
 * @brief SPRESENSE Arduino Software Serial library 
 * 
 * @details The Software Serial library has been developed to allow serial 
 *          communication on any digital pins of the SPRESENSE. It is possible
 *          to have 12 software serial ports operating at the same time with 
 *          speeds up to 250,000 bps.
 */

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <common/arm_internal.h>
extern "C" {
#include <cxd56_gpioint.h>
}
#include "wiring_private.h"
#include "utility.h"

/**
 * @brief Active listener.
 */
typedef struct __listener {
    SoftwareSerial *object;
    int irq;
} listener_t;

/**
 * @brief Table for 12 active listeners
 */
static listener_t _listeners[] = {
    { NULL, -1 },
    { NULL, -1 },
    { NULL, -1 },
    { NULL, -1 },
    { NULL, -1 },
    { NULL, -1 },
    { NULL, -1 },
    { NULL, -1 },
    { NULL, -1 },
    { NULL, -1 },
    { NULL, -1 },
    { NULL, -1 },
};

inline void SoftwareSerial::tunedDelay(unsigned long delay) { 
  /* following loop takes 4 cycles */
  do {
    __asm__ __volatile__("nop");
  } while(delay--);
}

bool SoftwareSerial::isActiveListener(SoftwareSerial *object)
{
  arrayForEach(_listeners, i) 
  {
    if (_listeners[i].object == object)
    {
      return true;
    }
  }

  return false;
}

bool SoftwareSerial::addActiveListener(SoftwareSerial *object, int irq)
{
  arrayForEach(_listeners, i) 
  {
    if (_listeners[i].object == NULL)
    {
      _listeners[i].object = object;
      _listeners[i].irq = irq;
      return true;
    }
  }

  return false;
}

void SoftwareSerial::removeActiveListener(SoftwareSerial *object)
{
  arrayForEach(_listeners, i) 
  {
    if (_listeners[i].object == object)
    {
      _listeners[i].object = NULL;
      _listeners[i].irq = -1;
    }
  }
}

SoftwareSerial *SoftwareSerial::findActiveListener(int irq)
{
  SoftwareSerial *object = NULL;

  arrayForEach(_listeners, i) 
  {
    if (_listeners[i].irq == irq)
    {
      object = _listeners[i].object;
    }
  }

  return object;
}

bool SoftwareSerial::listen()
{
  if (!isActiveListener(this))
  {
    _buffer_overflow = false;
    _receive_buffer_head = _receive_buffer_tail = 0;

    noInterrupts();
    int irq = cxd56_gpioint_config(pin_convert(_receivePin),
                                   GPIOINT_NOISE_FILTER_DISABLE |
                                   GPIOINT_PSEUDO_EDGE_FALL,
                                   SoftwareSerial::handle_interrupt,
                                   NULL);

    if (irq >= 0) 
    {
      cxd56_gpioint_enable(pin_convert(_receivePin));
      addActiveListener(this, irq);
    }
    interrupts();

    return true;
  }

  return false;
}

bool SoftwareSerial::stopListening()
{
  if (isActiveListener(this))
  {
    noInterrupts();
    cxd56_gpioint_disable(pin_convert(_receivePin));
    interrupts();
    removeActiveListener(this);
    return true;
  }
  return false;
}

void SoftwareSerial::recv()
{
  uint8_t d = 0;

  /* Wait approximately 1/2 of a bit width to "center" the sample */
  tunedDelay(_rx_delay_centering);

  /* Read each of the 8 bits */
  for (uint8_t i=8; i > 0; --i)
  {
    d >>= 1;
    if (getreg32(_receivePinRegAddr) & (1 << GPIO_INPUT_SHIFT))
    {
      d |= 0x80;
    }
    tunedDelay(_rx_delay_intrabit);
  }

  uint8_t next = (_receive_buffer_tail + 1) % SS_MAX_RX_BUFF;
  if (next != _receive_buffer_head)
  {
    /* save new data in buffer */
    _receive_buffer[_receive_buffer_tail] = d;
    _receive_buffer_tail = next;
  } 
  else 
  {
    _buffer_overflow = true;
  }
}

int inline SoftwareSerial::handle_interrupt(int irq, FAR void* context, FAR void *arg)
{
  SoftwareSerial *active_object;
  active_object = findActiveListener(irq);
  if (active_object)
  {
    active_object->recv();
  }

  return 0;
}

SoftwareSerial::SoftwareSerial(uint8_t receivePin, uint8_t transmitPin) : 
  _rx_delay_centering(0),
  _rx_delay_intrabit(0),
  _tx_delay(0),
  _buffer_overflow(false),
  _receive_buffer_tail(0),
  _receive_buffer_head(0)
{
  _transmitPin = transmitPin;
  _transmitPinRegAddr = get_gpio_regaddr(pin_convert(transmitPin));
  _receivePin = receivePin;
  _receivePinRegAddr = get_gpio_regaddr(pin_convert(receivePin));
}

SoftwareSerial::~SoftwareSerial()
{
  end();
}

void SoftwareSerial::begin(long speed)
{
  unsigned long bitDelay;

  pinMode(_transmitPin, OUTPUT);
  pinMode(_receivePin, INPUT_PULLUP);

  /* 4-cycle delays (must never be 0!) */
  bitDelay = (clockCyclesPerMicrosecond() * 250000) / speed;
  _tx_delay = bitDelay - 16;
  _rx_delay_centering = bitDelay + (bitDelay / 2) > 160 ? bitDelay + (bitDelay / 2) - 160 : 1;
  _rx_delay_intrabit = bitDelay - 16;

  listen();
}

void SoftwareSerial::end()
{
  stopListening();
}

int SoftwareSerial::read()
{
  if (!isListening())
  {
    return -1;
  }

  /* Empty buffer? */
  if (_receive_buffer_head == _receive_buffer_tail)
  {
    return -1;
  }

  /* Read from "head" and grab next byte */
  uint8_t d = _receive_buffer[_receive_buffer_head];
  _receive_buffer_head = (_receive_buffer_head + 1) % SS_MAX_RX_BUFF;
  return d;
}

int SoftwareSerial::available()
{
  if (!isListening())
  {
    return 0;
  }

  return (_receive_buffer_tail + SS_MAX_RX_BUFF - _receive_buffer_head) % SS_MAX_RX_BUFF;
}

size_t SoftwareSerial::write(uint8_t b)
{
  if (_tx_delay == 0)
  {
    setWriteError();
    return 0;
  }

  noInterrupts();
  putreg32(GPIO_OUTPUT_ENABLE | GPIO_OUTPUT_LOW, _transmitPinRegAddr);
  tunedDelay(_tx_delay);

  /* Write each of the 8 bits */
  for (uint8_t i = 8; i > 0; --i)
  {
    if (b & 1) 
    {
      putreg32(GPIO_OUTPUT_ENABLE | GPIO_OUTPUT_HIGH, _transmitPinRegAddr);
    }
    else
    {
      putreg32(GPIO_OUTPUT_ENABLE | GPIO_OUTPUT_LOW, _transmitPinRegAddr);
    }

    tunedDelay(_tx_delay);
    b >>= 1;
  }

  putreg32(GPIO_OUTPUT_ENABLE | GPIO_OUTPUT_HIGH, _transmitPinRegAddr);

  interrupts();
  tunedDelay(_tx_delay);
  
  return 1;
}

void SoftwareSerial::flush()
{
  /* There is no tx buffering, simply return */
}

int SoftwareSerial::peek()
{
  if (!isListening())
  {
    return -1;
  }

  /* Empty buffer? */
  if (_receive_buffer_head == _receive_buffer_tail)
  {
    return -1;
  }

  /* Read from "head" */
  return _receive_buffer[_receive_buffer_head];
}
