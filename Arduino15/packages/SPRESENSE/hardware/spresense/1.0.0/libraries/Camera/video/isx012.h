/****************************************************************************
 * nuttx/drivers/sensors/isx012.h
 *
 *   Copyright 2018 Sony Semiconductor Solutions Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef __INCLUDE_NUTTX_VIDEO_ISX012_H
#define __INCLUDE_NUTTX_VIDEO_ISX012_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sdk/config.h>
#include <nuttx/fs/ioctl.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define _IMGIOCBASE   (0x1100)

#define _IMGIOC(nr)       _IOC(_IMGIOCBASE,nr)

/**
 * @defgroup img_ioctl IOCTL commands
 * @{
 */

#define IMGIOC_SETSTATE   _IMGIOC(0x0001)
#define IMGIOC_SETMODE    _IMGIOC(0x0002)
#define IMGIOC_SETMODEP   _IMGIOC(0x0003)
#define IMGIOC_SETCISIF   _IMGIOC(0x0004)
#define IMGIOC_READREG    _IMGIOC(0x0005)
#define IMGIOC_WRITEREG   _IMGIOC(0x0006)
#define IMGIOC_MONIREF    _IMGIOC(0x0007)

enum isx012_state_e {
  STATE_ISX012_PRESLEEP,
  STATE_ISX012_SLEEP,
  STATE_ISX012_ACTIVE,
  STATE_ISX012_POWEROFF,
};

typedef enum isx012_state_e isx012_state_t;

enum isx012_format_e {
  FORMAT_ISX012_YUV,
  FORMAT_ISX012_RGB565,
  FORMAT_ISX012_JPEG_MODE1,
  FORMAT_ISX012_JPEG_MODE1_INT,
  FORMAT_ISX012_MAX
};

typedef enum isx012_format_e isx012_format_t;

enum isx012_rate_e {
  RATE_ISX012_120FPS = 0,
  RATE_ISX012_60FPS,
  RATE_ISX012_30FPS,
  RATE_ISX012_15FPS,
  RATE_ISX012_10FPS,
  RATE_ISX012_7_5FPS,
  RATE_ISX012_6FPS,
  RATE_ISX012_5FPS,
  RATE_ISX012_MAX,
};

typedef enum isx012_rate_e isx012_rate_t;

enum isx012_mode_e {
  MODE_ISX012_MONITORING,
  MODE_ISX012_CAPTURE,
  MODE_ISX012_HALFRELEASE,
};

typedef enum isx012_mode_e isx012_mode_t;

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct isx012_param_s
{
  isx012_format_t format;
  uint16_t        jpeg_hsize;
  uint16_t        jpeg_vsize;
  uint16_t        yuv_hsize;
  uint16_t        yuv_vsize;
  isx012_rate_t   rate;
};

typedef struct isx012_param_s isx012_param_t;

struct isx012_s
{
  isx012_param_t moni_param;
  isx012_param_t cap_param;
};

typedef struct isx012_s isx012_t;

struct isx012_dev_s
{
  FAR struct i2c_master_s *i2c; /* I2C interface */
  uint8_t  addr;                /* I2C address */
  int      freq;                /* Frequency */
  isx012_t image;               /* image param */
  sem_t wait;
};

typedef struct isx012_dev_s isx012_dev_t;

struct isx012_reg_s {
  uint16_t regaddr;
  uint16_t regval;
  uint8_t  regsize;
};

typedef struct isx012_reg_s isx012_reg_t;

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifdef __cplusplus
#  define EXTERN extern "C"
extern "C"
{
#else
#  define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

int isx012_initialize(isx012_dev_t *priv);
int isx012_open( void );
int isx012_close( void );
int isx012_ioctl(int cmd, unsigned long arg);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __INCLUDE_NUTTX_VIDEO_ISX012_H */
