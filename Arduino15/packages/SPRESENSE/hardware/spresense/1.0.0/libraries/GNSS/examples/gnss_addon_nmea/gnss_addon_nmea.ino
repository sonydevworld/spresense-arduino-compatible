/*
 *  gnss_addon_nmea.ino - GNSS Add-on NMEA example application
 *  Copyright 2023 Sony Semiconductor Solutions Corporation
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
 * @file gnss_addon_nmea.ino
 * @author Sony Semiconductor Solutions Corporation
 * @brief GNSS Add-on NMEA example application
 * @details This skecth provides an example of NMEA output on GNSS Addon board.
 */

#include <GNSS.h>
#include <gpsutils/cxd56_gnss_nmea.h>

SpGnssAddon Gnss;
GnssPositionData2 PositionData;

/* NMEA buffer */
static char nmea_buf[NMEA_SENTENCE_MAX_LEN];

/* NMEA callback functions */
static char *reqbuf(uint16_t size)
{
  /* Get the pointer to NMEA buffer */
  if (size > sizeof(nmea_buf)) {
    return NULL;
  }
  return nmea_buf;
}

static void freebuf(char *buf)
{
  /* do nothing */
}

static int outbin(char *buf, uint32_t len)
{
  /* unused dummy function */
  return len;
}

static int outnmea(char *buf)
{
  /* Output NMEA to serial monitor */
  return Serial.print(buf);
}

void setup()
{
  /* Initialize Serial */
  Serial.begin(115200);

  /* Initialize GNSS */
  if (Gnss.begin()) {
    Serial.println("begin error!");
  }

  /* Set interval */
  Gnss.setInterval(SpInterval_1Hz);

  /* Start GNSS */
  if (Gnss.start()) {
    Serial.println("start error!");
  }

  /* Initialize NMEA library */
  NMEA_InitMask2();

  /* Select NMEA sentence */
  NMEA_SetMask2(NMEA_GGA_ON |
                NMEA_GLL_ON |
                NMEA_GSA_ON |
                NMEA_GSV_ON |
                NMEA_GNS_ON |
                NMEA_RMC_ON |
                NMEA_VTG_ON |
                NMEA_ZDA_ON);

  /* Register callbacks to output NMEA */
  NMEA_OUTPUT_CB  funcs;
  funcs.bufReq  = reqbuf;
  funcs.out     = outnmea;
  funcs.outBin  = outbin;
  funcs.bufFree = freebuf;
  NMEA_RegistOutputFunc2(&funcs);
}

void loop()
{
  /* Wait for GNSS data to be updated */
  if (Gnss.waitUpdate(-1)) {
    /* Get the GNSS data */
    Gnss.getPositionData(&PositionData);
    /* Convert to NMEA */
    NMEA_Output2(&PositionData.Data);
  }
}

