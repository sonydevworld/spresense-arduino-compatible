/*
  wiring_analog.c - analog I/O for the Sparduino SDK
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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <common/up_arch.h>
#include <nuttx/config.h>
#include <sdk/config.h>
#include <nuttx/drivers/pwm.h>
#include <cxd56_pinconfig.h>
#include <cxd56_adc.h>
#include <cxd56_clock.h>
#include <arch/chip/cxd56_scu.h>
#include <arch/chip/cxd56_adc.h>
#include <Arduino.h>
#include "utility.h"
#include "wiring_private.h"

#ifndef CONFIG_CXD56_PWM0
# error Please enable PWM0 in NuttX
#endif // CONFIG_CXD56_PWM0

#ifndef CONFIG_CXD56_PWM1
# error Please enable PWM1 in NuttX
#endif // CONFIG_CXD56_PWM1

#ifndef CONFIG_CXD56_PWM2
# error Please enable PWM2 in NuttX
#endif // CONFIG_CXD56_PWM2

#ifndef CONFIG_CXD56_PWM3
# error Please enable PWM3 in NuttX
#endif // CONFIG_CXD56_PWM3

#ifndef CONFIG_CXD56_ADC
# error Please enable ADC in NuttX
#endif // CONFIG_CXD56_ADC

#ifndef CONFIG_CXD56_HPADC0
# error Please enable HPADC0 in NuttX
#endif // CONFIG_CXD56_HPADC0

#ifndef CONFIG_CXD56_HPADC1
# error Please enable HPADC1 in NuttX
#endif // CONFIG_CXD56_HPADC1

#ifndef CONFIG_CXD56_LPADC_ALL
# error Please enable LPADC ALL in NuttX
#endif // CONFIG_CXD56_LPADC_ALL

#define ANALOG_TIMER_ID         CXD56_TIMER1
#define ANALOG_TIMER_FD_INVALID (-1)
#define ANALOG_TIMER_DEV_NAME   "/dev/timer1"

#define PWM0_DEVPATH	"/dev/pwm0"
#define PWM1_DEVPATH	"/dev/pwm1"
#define PWM2_DEVPATH	"/dev/pwm2"
#define PWM3_DEVPATH	"/dev/pwm3"

#define ANALOG_FREQUENCY    (490) // Hz
//#define ANALOG_FREQUENCY    (50)
#define GET_ON_DURATION(__duty, __freq)     ((__duty) * 1000000L / (__freq) / 255)
#define GET_OFF_DURATION(__duty, __freq)    ((255 - (__duty)) * 1000000L / (__freq) / 255)

#define BIT_STATE   (0)

#define F_RUNNING   (1)

#define SET_RUNNING(f)  bitSet((f), BIT_STATE)
#define SET_PAUSED(f)   bitClear((f), BIT_STATE)
#define IS_RUNNING(f)   ((f) & F_RUNNING)

#define DUTY_CONVERT(d) (d * 65535 / 255)

typedef struct {
    uint8_t pin;
    uint8_t duty;
    uint16_t running:1;
    uint32_t pin_addr;
    uint32_t freq;
    uint32_t on_duration;   // us
    uint32_t off_duration;  // us
    uint64_t expire;        // us
} analog_timer_info_t;

typedef struct pwm_lowerhalf_s pwm_dev_t;
typedef struct {
    uint8_t pin;
    uint8_t duty;
    uint16_t running:1;
    uint32_t pulse_width;
    uint32_t freq;
    int fd;
} pwm_timer_info_t;

typedef struct {
    uint8_t pin;
    uint8_t running:1;
    int16_t average;
    const char* dev_path;
} adc_t;

static analog_timer_info_t s_sim_timers[] __attribute__((aligned (4))) = {
    /* pin,  dut, run, adr, freq, on, off, exp */
    { PIN_D00, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D01, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D02, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D04, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D07, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D08, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D10, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D11, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D12, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D13, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D14, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D15, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D16, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D17, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D18, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D19, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D20, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D21, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D22, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D23, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D24, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D25, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D26, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D27, 0,   0,   0,   0,   0,   0,   0 },
    { PIN_D28, 0,   0,   0,   0,   0,   0,   0 },
};

static pwm_timer_info_t s_pwm_timers[] __attribute__((aligned (4))) = {
    /* pin,      duty,  run,   width, freq,  dev */
    { PIN_PWM_0, 0,     0,     0,     0,     0 },
    { PIN_PWM_1, 0,     0,     0,     0,     0 },
    { PIN_PWM_2, 0,     0,     0,     0,     0 },
    { PIN_PWM_3, 0,     0,     0,     0,     0 },
};

static adc_t s_adcs[] __attribute__((aligned (4))) = {
    /* pin,   run, avg, dev */
    { PIN_A0, 0,   0,   "/dev/lpadc0" },
    { PIN_A1, 0,   0,   "/dev/lpadc1" },
    { PIN_A2, 0,   0,   "/dev/lpadc2" },
    { PIN_A3, 0,   0,   "/dev/lpadc3" },
    { PIN_A4, 0,   0,   "/dev/hpadc0" },
    { PIN_A5, 0,   0,   "/dev/hpadc1" },
};

static int s_timer_fd = -1;

static int sim_pin2slot(uint8_t pin)
{
  arrayForEach(s_sim_timers, i) {
    if (s_sim_timers[i].pin == pin) {
      return i;
    }
  }

  return -1;
}

static void sim_set_timer_info(analog_timer_info_t* info, uint8_t duty, uint32_t pulse_width, uint32_t freq)
{
  assert(info);

  info->duty = duty;
  info->running = true;
  info->freq = freq;
  info->on_duration = pulse_width;
  info->off_duration = GET_OFF_DURATION(duty, freq);
  info->expire = micros() + info->on_duration;
}

static int sim_prepare_timer(void)
{
  int ret = OK;

  if (s_timer_fd < 0) {
    ret = util_open_timer(ANALOG_TIMER_DEV_NAME, &s_timer_fd);
    arrayForEach(s_sim_timers, i) {
      s_sim_timers[i].pin_addr = get_gpio_regaddr((uint32_t)pin_convert(s_sim_timers[i].pin));
    }
  }

  return ret;
}

static uint64_t sim_get_next_expire(void)
{
  uint64_t next_expire = UINT64_MAX;

  arrayForEach(s_sim_timers, i) {
    if (s_sim_timers[i].running && s_sim_timers[i].expire < next_expire) {
      next_expire = s_sim_timers[i].expire;
    }
  }

  return next_expire == UINT64_MAX ? 0 : next_expire;
}

static bool sim_timer_handler(FAR uint32_t *next_interval_us, FAR void *arg)
{
  static bool ret;
  static uint64_t now;
  static uint64_t next_expire;
  static uint32_t reg_val;
  static const uint32_t mask = 1 << GPIO_OUTPUT_SHIFT;

  unuse(arg);

  next_expire = UINT64_MAX;
  now = micros();

  //printf("timer handler called at %llu us\n", now);
  arrayForEach(s_sim_timers, i) {
    if (!s_sim_timers[i].running) {
      continue;
    }

    if (s_sim_timers[i].expire <= now) {
      reg_val = getreg32(s_sim_timers[i].pin_addr);
      s_sim_timers[i].expire = now + ((reg_val & mask) ? s_sim_timers[i].off_duration : s_sim_timers[i].on_duration);
      reg_val ^= mask;
      putreg32(reg_val, s_sim_timers[i].pin_addr);
      //printf("timer [%d] will expire at %llu, pin value = %d\n", i, s_sim_timers[i].expire, bitRead(reg_val, GPIO_OUTPUT_SHIFT));
    }

    if (s_sim_timers[i].expire < next_expire) {
      next_expire = s_sim_timers[i].expire;
    }
  }

  //printf("next expire = %llu\n", next_expire);
  if (next_expire != UINT64_MAX) {
    *next_interval_us = next_expire - now;
    ret = true;
    //printf("reschedule analog timer %u\n", *next_interval_us);
  } else {
    ret = false;
  }

  return ret;
}

static void sim_start(void)
{
  uint64_t expire;
  uint64_t now;
  uint32_t timeout;

  (void) util_stop_timer(s_timer_fd);

  expire = sim_get_next_expire();
  now = micros();
  timeout = (uint32_t)(expire - now);

  while (now > expire) {
    //printf("timer interrupt missed, handle it first\n");
    sim_timer_handler(&timeout, NULL);
    expire = sim_get_next_expire();
    now = micros();
  }

  //printf("analog timer next expire %llu from now %lu, timeout = %lu\n", expire, now, timeout);
  (void) util_start_timer(s_timer_fd, timeout, sim_timer_handler);
}

static void sim_stop(uint8_t pin)
{
  int slot = sim_pin2slot(pin);

  if (slot >= 0) {
    noInterrupts();
    s_sim_timers[slot].running = false;
    interrupts();
  }
}

static void sim_write(uint8_t pin, uint32_t pulse_width, uint32_t freq)
{
  int slot = sim_pin2slot(pin);
  int value;

  if (slot < 0) {
    printf("ERROR: Invalid pin number [%u]\n", pin);
    return;
  }

  if (sim_prepare_timer()) {
    return;
  }

  pinMode(pin, OUTPUT);
  value = 255 * pulse_width * freq / 1000000L;

  if (value <= 0) {
      digitalWrite(pin, LOW);
  } else if (value >= 255) {
      digitalWrite(pin, HIGH);
  } else {
    if (s_sim_timers[slot].running && s_sim_timers[slot].duty == value) {
      return; // nothing changed
    }

    digital_write(pin, HIGH, false);
    sim_set_timer_info(&s_sim_timers[slot], value, pulse_width, freq);
    sim_start();
  }
}

static int pwm_pin2slot(uint8_t pin)
{
  arrayForEach(s_pwm_timers, i) {
    if (pin == s_pwm_timers[i].pin) {
      return i;
    }
  }

  return -1;
}

static void pwm_prepare_timer(uint8_t pin)
{
  int slot = pwm_pin2slot(pin);

  if (slot >= 0 && s_pwm_timers[slot].fd == 0) {
    s_pwm_timers[slot].fd = open(pin == PIN_PWM_0 ? PWM0_DEVPATH :
                                 pin == PIN_PWM_1 ? PWM1_DEVPATH :
                                 pin == PIN_PWM_2 ? PWM2_DEVPATH : PWM3_DEVPATH, O_RDONLY);
    assert(s_pwm_timers[slot].fd);
  }
}

static void pwm_set_timer_info(uint8_t pin, uint32_t pulse_width, uint32_t freq)
{
  int slot = pwm_pin2slot(pin);

  if (slot >= 0) {
    s_pwm_timers[slot].pin = pin;
    s_pwm_timers[slot].freq = freq;
    s_pwm_timers[slot].duty = 255 * pulse_width * freq / 1000000L;
  }
}

static void pwm_start(uint8_t pin)
{
  int slot = pwm_pin2slot(pin);
  int ret;

  if (slot < 0) {
    return;
  }

  struct pwm_info_s info = {
    .frequency = s_pwm_timers[slot].freq,
    .duty = DUTY_CONVERT(s_pwm_timers[slot].duty)
  };

  ret = ioctl(s_pwm_timers[slot].fd, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t)&info));
  if (ret != OK) {
    printf("ioctl(PWMIOC_SETCHARACTERISTICS) failed (errno = %d)\n", errno);
    return;
  }

  ret = ioctl(s_pwm_timers[slot].fd, PWMIOC_START, 0);
  if (ret != OK) {
    printf("ioctl(PWMIOC_START) failed (errno = %d)\n", errno);
    return;
  }

  s_pwm_timers[slot].running = true;
}

static void pwm_stop(uint8_t pin)
{
  int slot = pwm_pin2slot(pin);
  int ret;

  if (slot < 0) {
    return;
  }

  if (s_pwm_timers[slot].running) {
    ret = ioctl(s_pwm_timers[slot].fd, PWMIOC_STOP, 0);
    if (ret != OK) {
      printf("ioctl(PWMIOC_STOP) failed (errno = %d)\n", errno);
      return;
    }

    s_pwm_timers[slot].duty = 0;
    s_pwm_timers[slot].freq = 0;
    s_pwm_timers[slot].pulse_width = 0;
    s_pwm_timers[slot].running = false;

    close(s_pwm_timers[slot].fd);
    
    s_pwm_timers[slot].fd = 0;
  }
}

static void pwm_write(uint8_t pin, uint32_t pulse_width, uint32_t freq)
{
  int slot = pwm_pin2slot(pin);

  if (slot < 0) {
    return;
  }
  
  if (pulse_width==0){
    if (s_pwm_timers[slot].running){
      pwm_stop(pin);
    }
    return;
  }

  if (s_pwm_timers[slot].running &&
      s_pwm_timers[slot].pulse_width == pulse_width) {
    return;
  }

  pwm_prepare_timer(pin);
  pwm_set_timer_info(pin, pulse_width, freq);
  pwm_start(pin);
}

void analog_stop(uint8_t pin)
{
  if (pin == PIN_PWM_0 || pin == PIN_PWM_1 ||
      pin == PIN_PWM_2 || pin == PIN_PWM_3) {
    pwm_stop(pin);
  } else {
    sim_stop(pin);
  }
}

void analog_write(uint8_t pin, uint32_t pulse_width, uint32_t freq)
{
  if (pin == PIN_PWM_0 || pin == PIN_PWM_1 ||
      pin == PIN_PWM_2 || pin == PIN_PWM_3) {
    pwm_write(pin, pulse_width, freq);
  } else {
    sim_write(pin, pulse_width, freq);
  }
}

/*
 * Reference voltage is a fixed value which is depending on the board.
 * e.g.)
 * - Reference Voltage of A4 and A5 pins on Main Board is 0.7V.
 * - Reference Voltage of A0 ~ A5 pins on External Interface board
 *   is selected 3.3V or 5.0V by a IO Volt jumper pin.
 */
void analogReference(uint8_t mode)
{
  /* Not supported, this function is kept only for compatibility. */
  unuse(mode);
}

static int ad_pin_fd[6]= {-1,-1,-1,-1,-1,-1};
int analogRead(uint8_t pin)
{
  int ret = 0;
  int errval = 0;
  int fd;
  ssize_t nbytes = 0;

  uint8_t aidx = _PIN_OFFSET(pin);
  if ((pin < PIN_A0) || (pin > PIN_A5)) {
    printf("ERROR: Invalid pin number [%u]\n", pin);
    printf("pin must be specified as A0 to A5\n");
    return 0;
  }
  if (s_adcs[aidx].running) {
    printf("ERROR: Already in progress A%u\n", aidx);
    return 0;
  }

  if (ad_pin_fd[aidx] < 0) {
      fd = open(s_adcs[aidx].dev_path, O_RDONLY);
      if (fd < 0) {
          printf("ERROR: Failed to open adc device,%d\n", errno);
          goto out;
      }

      /* Change adc running */

      s_adcs[aidx].running = true;

      /* SCU FIFO overwrite */

      if (ioctl(fd, SCUIOC_SETFIFOMODE, 1) < 0) {
        printf("ERROR: Failed to set SCU FIFO mode\n");
        goto out;
      }

      /* ADC FIFO size */

      if (ioctl(fd, ANIOC_CXD56_FIFOSIZE, 2) < 0) {
        printf("ERROR: Failed to set ADC FIFO size\n");
        goto out;
      }

      /* start ADC */

      if (ioctl(fd, ANIOC_CXD56_START, 0) < 0) {
        printf("ERROR: Failed to start ADC\n");
        goto out;
      }
      ad_pin_fd[aidx] = fd;
  } else {
      fd = ad_pin_fd[aidx];
  }

  /* read data */

  int16_t sample;

  do {
    nbytes = read(fd, &sample, sizeof(sample));
    //printf("nbytes=%d\n", nbytes);
    if (nbytes < 0) {
      errval = errno;
      printf("read failed:%d\n", errval);
      goto out;
    }
  } while (nbytes == 0);

  ret = map(sample, SHRT_MIN, SHRT_MAX, 0, 1023);

out:
#if 0
  /* To realize the faster read operation from the ADC,
   * Do not stop and close once opened.
   */
  if (fd >= 0) {
    (void) ioctl(fd, ANIOC_CXD56_STOP, 0);
    (void) close(fd);
  }
#endif

  s_adcs[aidx].running = false;

  return ret;
}

void analogWrite(uint8_t pin, int value)
{
  value = value < 0   ? 0   :
          value > 255 ? 255 : value;
  analog_write(pin, GET_ON_DURATION(value, ANALOG_FREQUENCY), ANALOG_FREQUENCY);
}
