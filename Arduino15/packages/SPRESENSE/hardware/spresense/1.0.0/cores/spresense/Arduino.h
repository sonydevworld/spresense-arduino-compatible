/*
  Arduino.h - Main include file for the Spresense SDK
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

#ifndef Arduino_h
#define Arduino_h

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <nuttx/config.h>
#include <sdk/config.h>
#include <math.h>

// Some libraries assumes that AVR-specific definitions are
// automatically included from Arduino.h... Therefore, here
// includes the following dummy avr files.

#include "avr/pgmspace.h"
#include "avr/interrupt.h"

#include "binary.h"
//#include "itoa.h"

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include "wiring.h"
#include "leds.h"

typedef uint8_t byte;
typedef uint16_t word;

/* Constants */
#define LOW     0x0
#define HIGH    0x1
#define CHANGE  0x2
#define RISING  0x3
#define FALLING 0x4

#define INPUT           0x0
#define OUTPUT          0x1
#define INPUT_PULLUP    0x2
#define INPUT_PULLDOWN  0x3

#define LSBFIRST 0x0
#define MSBFIRST 0x1

#define PI              M_PI
#define HALF_PI         M_PI_2
#define TWO_PI          (PI * 2)
#define DEG_TO_RAD      0.017453292519943295769236907684886
#define RAD_TO_DEG      57.295779513082320876798154814105
#define EULER           M_E

/* Digital I/O */
void pinMode(uint8_t, uint8_t);
void digitalWrite(uint8_t, uint8_t);
int digitalRead(uint8_t);

/* Analog I/O */
void analogReference(uint8_t mode);
int analogRead(uint8_t);
void analogReadMap(uint8_t pin, int16_t min, int16_t max);
void analogWriteSetDefaultFreq(uint32_t);
uint32_t analogWriteGetDefaultFreq(void);
void analogWriteFreq(uint8_t, int, uint32_t);
void analogWrite(uint8_t, int);

/* Advanced I/O */
void tone(uint8_t pin, unsigned int frequency, unsigned long duration);
void noTone(uint8_t pin);
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout);
unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout);

/* Time */
uint64_t millis(void);
uint64_t micros(void);
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);    // can be accurate if us >= 8
unsigned long clockCyclesPerMicrosecond(void);
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
#define microsecondsToClockCycles(a) ( (unsigned long long) (a) * clockCyclesPerMicrosecond() )

/* Math */
#define min(a, b)    ((a) < (b) ? (a) : (b))
#define max(a, b)    ((a) > (b) ? (a) : (b))
#define abs(x)       ((x) > 0 ? (x) : -(x))
#define round(x)     ((x) >= 0 ? (long)((x) + 0.5) : (long)((x) - 0.5))
#define radians(deg) ((deg) * DEG_TO_RAD)
#define degrees(rad) ((rad) * RAD_TO_DEG)
#define sq(x)        ((x) * (x))
#define constrain(amt, low, high) \
   ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
// double sqrt(double x) // math.h
// double pow(double b, double e) // math.h

/* Trigonometry */
// double sin(double x); // math.h
// double cos(double x); // math.h
// double tan(double x); // math.h

/* Bits and Bytes*/
#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#define bit(b) (1UL << (b))
//macro added for compatibility
#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif
#ifndef cbi
#define cbi(reg, bit) (*(reg) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(reg, bit) (*(reg) |= _BV(bit))
#endif

/* Interrupts */
void interrupts(void);
void noInterrupts(void);

/* sketch */
void init(void);
void initVariant(void);
int atexit(void (*func)());
void setup(void);
void loop(void);
long map(long x, long in_min, long in_max, long out_min, long out_max);
void yield(void);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#ifdef __cplusplus
#include "WCharacter.h" /* Characters */
#include "WString.h"
#endif // __cplusplus

// Include board variant
#if 0
#include "variant.h"
#include "watchdog.h"
#endif

#ifdef __cplusplus
/* Advanced I/O */
void tone(uint8_t pin, unsigned int frequency, unsigned long duration = 0);
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);
unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);

/* WMath Prototypes */
uint16_t makeWord(uint16_t w);
uint16_t makeWord(byte h, byte l);
#define word(...) makeWord(__VA_ARGS__)
long random(long __max);
long random(long __min, long __max);
void randomSeed(unsigned long seed);

/* External Interrupts */
void attachInterrupt(uint8_t interrupt, void (*isr)(void), int mode);
void detachInterrupt(uint8_t interrupt);
#define digitalPinToInterrupt(p) (p) /* treat pin number as the interrupt number */

/* Timer Interrupt */
void attachTimerInterrupt(unsigned int (*isr)(void), unsigned int us);
// Parameter:
//   isr: the function to call when the timer interrupt occurs.
//        This function must return the next timer period [microseconds].
//        If this function returns 0, the timer stops and it behaves as oneshot timer.
//   us: microseconds.
//       The maximum value is about 26 seconds and if it exceeds, an error occurs.
// Note:
//   This can not be used at the same time with tone().
void detachTimerInterrupt(void);

/* macro to customize heap size for subcore */
#define USER_HEAP_SIZE(size) __asm (".global __userheap_size__; .equ __userheap_size__," #size);

#endif // __cplusplus

#include <HardwareSerial.h>

#endif // Arduino_h
