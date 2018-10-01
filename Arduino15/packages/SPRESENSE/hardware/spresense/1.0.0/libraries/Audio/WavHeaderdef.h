/*
 *  WavHeaderdef.h - Wav header structure definition
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

#ifndef WavHeaderdef_h
#define WavHeaderdef_h

/* Wav header constant value definitions */

#define CHUNKID_RIFF        ("RIFF")
#define FORMAT_WAVE         ("WAVE")
#define SUBCHUNKID_FMT      ("fmt ")
#define SUBCHUNKID_DATA     ("data")
#define AUDIO_FORMAT_PCM    (0x0001)
#define FMT_SIZE            (0x10)

/**
 * @brief Wav file Header definition
 */

struct WavFormat_t
{
  WavFormat_t () :
     fmt_size(FMT_SIZE)
   , format(AUDIO_FORMAT_PCM)
  {
    memcpy(riff, CHUNKID_RIFF, strlen(CHUNKID_RIFF));
    memcpy(wave, FORMAT_WAVE, strlen(FORMAT_WAVE));
    memcpy(fmt, SUBCHUNKID_FMT, strlen(SUBCHUNKID_FMT));
    memcpy(data, SUBCHUNKID_DATA, strlen(SUBCHUNKID_DATA));
  }

  uint8_t   riff[4];    /**< RIFF header. Must be "RIFF" */
  uint32_t  total_size; /**< Whole file size - 8*/
  uint8_t   wave[4];    /**< WAV header. Must be "WAVE" */
  uint8_t   fmt[4];     /**< fmt chunk id. Must be "fmt " (<- including a blank) */
  uint32_t  fmt_size;   /**< fmt chunk size */
  uint16_t  format;     /**< format type */
  uint16_t  channel;    /**< Channel num */
  uint32_t  rate;       /**< Sampling rate */
  uint32_t  avgbyte;    /**< Sampling rate * Byte length * Channel num */
  uint16_t  block;      /**< channel num * Byte length */
  uint16_t  bit;        /**< Bit length */
  uint8_t   data[4];    /**< data chunk id. Must be "data" */
  uint32_t  data_size;  /**< data chunk size */
};

#endif // WavHeaderdef_h

