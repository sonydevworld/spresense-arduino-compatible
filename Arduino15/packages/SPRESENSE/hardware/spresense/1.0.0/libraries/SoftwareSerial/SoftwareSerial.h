/*
 *  SoftwareSerial.h - Spresense Arduino Software Serial library
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

#ifndef SoftwareSerial_h
#define SoftwareSerial_h

/**
 * @file SoftwareSerial.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief SPRESENSE Arduino Software Serial library 
 * 
 * @details The Software Serial library has been developed to allow serial 
 *          communication on any digital pins of the SPRESENSE. It is possible
 *          to communicate with speeds up to 250,000 bps.
 */

/**
 * @defgroup sw_serial Software Serial Library API
 * @brief API for using Software Serial
 * @{
 */

#include <inttypes.h>
#include <Stream.h>

#ifndef SS_MAX_RX_BUFF
#define SS_MAX_RX_BUFF 64 /**< Receive buffer size */
#endif

/**
 * @class SoftwareSerial
 * @brief SoftwareSerial controller
 *
 * @details You can control Software Serial comunication by operating SoftwareSerial 
 *          objects instantiated in your app. It is usable on all the GPIO pins 
 *          (D0 - D28).
 */
class SoftwareSerial : public Stream
{
private:
  uint8_t _receivePin;           /**< Receive pin */
  uint8_t _transmitPin;          /**< Transmit pin */
  uint32_t _receivePinRegAddr;   /**< Receive pin register address */
  uint32_t _transmitPinRegAddr;  /**< Transmit pin register address */

  unsigned long _rx_delay_centering;  /**< 4-cycle delays to center the sample (must never be 0!) */
  unsigned long _rx_delay_intrabit;   /**< 4-cycle delays to get next receive bit (must never be 0!) */
  unsigned long _tx_delay;            /**< 4-cycle delays to start transmit next bit (must never be 0!) */

  uint16_t _buffer_overflow:1;  /**< Overflow status */

  uint8_t _receive_buffer[SS_MAX_RX_BUFF];  /**< Receive buffer size */
  volatile uint8_t _receive_buffer_tail;    /**< Receive buffer tail */
  volatile uint8_t _receive_buffer_head;    /**< Receive buffer head */

  /**
   * @brief The receive routine called by the interrupt handler
   */
  inline void recv();

  /**
   * @brief Delay
   * 
   * @param [in] delay 4-cycle delays
   */
  static inline void tunedDelay(unsigned long delay);

  /**
   * @brief Checks if the SoftwareSerial object is active listener
   * 
   * @param [in] object SoftwareSerial object to check
   * @return true if object is active listener, -1 if not
   */
  static bool isActiveListener(SoftwareSerial *object);

  /**
   * @brief Add the SoftwareSerial object to active listeners
   * 
   * @param [in] object SoftwareSerial object to add
   * @param [in] irq Interrupt number
   * @return true if success, false if failure
   */
  static bool addActiveListener(SoftwareSerial *object, int irq);

  /**
   * @brief Remove the SoftwareSerial object from active listeners
   * 
   * @param [in] object SoftwareSerial object to remove
   */
  static void removeActiveListener(SoftwareSerial *object);

  /**
   * @brief Get the SoftwareSerial object
   * 
   * @param [in] irq Interrupt number
   * @return SoftwareSerial object
   */
  static SoftwareSerial *findActiveListener(int irq);

public:
  /**
   * @brief Create SoftwareSerial object
   * 
   * @param [in] receivePin Pin used for receive
   * @param [in] transmitPin Pin used for transmit
   */
  SoftwareSerial(uint8_t receivePin, uint8_t transmitPin);

  /**
   * @brief Destroy SoftwareSerial object
   */
  ~SoftwareSerial();

  /**
   * @brief Initialize the serial communication
   * 
   * @param [in] speed Baud rate. The maximum speed is 250,000 bps.
   */
  void begin(long speed);

  /**
   * @brief Deinitialize the serial communication
   */
  void end();

  /**
   * @brief Sets the current object as the "listening" one
   * 
   * @return true if success, false if failure
   */
  bool listen();

  /**
   * @brief Checks if the SoftwareSerial object is listening
   * 
   * @return true if is listening
   */
  bool isListening() { return isActiveListener(this); }

  /**
   * @brief Stop listening.
   * 
   * @return true if we were actually listening
   */
  bool stopListening();

  /**
   * @brief Checks if a SoftwareSerial buffer overflow has occurred.
   * 
   * @return true if overflow
   */
  bool overflow() { bool ret = _buffer_overflow; if (ret) _buffer_overflow = false; return ret; }

  /**
   * @brief Get a character that was received on the RX pin of the SoftwareSerial object.
   * 
   * @details Subsequent calls to this function will return the same character.
   * @return the character read, or -1 if none is available 
   */
  int peek();

  /**
   * @brief Send a single byte to the transmit pin of SoftwareSerial object
   * 
   * @param [in] byte Byte to write
   * @return Return the number of bytes written
   */
  virtual size_t write(uint8_t byte);

  /**
   * @brief Read data from buffer
   * 
   * @return The byte read, or -1 if none is available 
   */
  virtual int read();

  /**
   * @brief Get the number of bytes available for reading from a SoftwareSerial objects
   * 
   * @return number of bytes available to read 
   */
  virtual int available();

  /**
   * @brief There is no tx buffering, simply return
   */
  virtual void flush();
  
  using Print::write;

  /**
   * @brief Interrupt handling
   * 
   * @param [in] irq Interrupt number
   * @param [in] context Context (not used)
   * @param [in] arg Argument (not used)
   */
  static inline int handle_interrupt(int irq, FAR void* context, FAR void *arg);
};

/** @} sw_serial */

#endif
