/*
 *  gnss_nmea.cpp - NMEA's GGA sentence
 *  Copyright 2017 Sony Semiconductor Solutions Corporation
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
 * @file gnss_nmea.cpp
 * @author Sony Corporation
 * @brief NMEA's GGA sentence
 */

/* include the GNSS library */
#include <GNSS.h>

#define CORIDNATE_TYPE_LATITUDE   0  /**< Coordinate type latitude */
#define CORIDNATE_TYPE_LONGITUDE  1  /**< Coordinate type longitude */

#define STRING_BUFFER_SIZE  128      /**< Sentence buffer size */

/**
 * @brief Calculate the checksum and add it to the end.
 * 
 * @param [in] pStrDest Data to calculate checksum
 * @return checksum
 */
static unsigned short CalcCheckSum(const char *pStrDest)
{
  unsigned short CheckSum = 0;
  int cnt;

  /* Calculate checksum as xor of characters. */
  for (cnt = 1; pStrDest[cnt] != 0x00; cnt++)
  {
    CheckSum = CheckSum ^ pStrDest[cnt];
  }

  return CheckSum;
}

/**
 * @brief Convert coordinate values for NMEA.
 * 
 * @param [out] pBuffer %Buffer to write converted values
 * @param [in] length Size of pBuffer
 * @param [in] Coordinate Latitude or longitude
 * @param [in] cordinate_type Coordinate type: CORIDNATE_TYPE_LATITUDE or CORIDNATE_TYPE_LONGITUDE
 */
static void CoordinateToString(char *pBuffer, int length, double Coordinate,
                               unsigned int cordinate_type)
{
  double tmp;
  int Degree;
  int Minute;
  int Minute2;
  char direction;
  unsigned char fixeddig;

  const static struct {
    unsigned char fixeddigit;
    char dir[2];
  } CordInfo[] = {
     { .fixeddigit = 2, .dir = { 'N', 'S' } },
     { .fixeddigit = 3, .dir = { 'E', 'W' } },
  };

  if (cordinate_type > CORIDNATE_TYPE_LONGITUDE)
  {
    snprintf(pBuffer, length, ",,");
    return;
  }

  if (Coordinate >= 0.0)
  {
    tmp = Coordinate;
    direction = CordInfo[cordinate_type].dir[0];
  }
  else
  {
    tmp = -Coordinate;
    direction = CordInfo[cordinate_type].dir[1];
  }
  fixeddig = CordInfo[cordinate_type].fixeddigit;
  Degree = (int) tmp;
  tmp = (tmp - (double)Degree) * 60;
  Minute = (int)tmp;
  tmp = (tmp - (double)Minute) * 10000 + 0.5;
  Minute2 = (int)tmp;

  snprintf(pBuffer, length, "%0*d%02d.%04d,%c,", fixeddig, Degree, Minute, Minute2, direction);

  return ;
}

String getNmeaGga(SpNavData* pNavData)
{
  String Gga;
  char StringBuffer[STRING_BUFFER_SIZE];
  int msec;
  unsigned short CheckSum;

  /* Set Header. */
  Gga = "$GPGGA,";

  /* Set time. */
  msec = pNavData->time.usec / 10000;
  snprintf(StringBuffer, STRING_BUFFER_SIZE, "%02d%02d%02d.%02d,", pNavData->time.hour, pNavData->time.minute, pNavData->time.sec, msec);
  Gga += StringBuffer;

  /* Set Coordinate. */
  if (pNavData->posDataExist)
  {
    /* Convert to DMM(Degree,Minute,Minute). */
    CoordinateToString(StringBuffer, STRING_BUFFER_SIZE, pNavData->latitude, CORIDNATE_TYPE_LATITUDE);
    Gga += StringBuffer;

    CoordinateToString(StringBuffer, STRING_BUFFER_SIZE, pNavData->longitude, CORIDNATE_TYPE_LONGITUDE);
    Gga += StringBuffer;
  }
  else
  {
    /* Position not fixed. */
    snprintf(StringBuffer, STRING_BUFFER_SIZE, ",,,,");
    Gga += StringBuffer;
  }

  /* Set Quality indicator. */
  if (pNavData->type != SpPvtTypeGnss)
  {
    /* Fix invalid. */
    snprintf(StringBuffer, STRING_BUFFER_SIZE, "0,");
  }
  else
  {
    /* Set fixed value. */
    /* GPS SPS mode,fix valid. */
    snprintf(StringBuffer, STRING_BUFFER_SIZE, "1,");
  }
  Gga += StringBuffer;

  /* Set Number of satellites in use. */
  snprintf(StringBuffer, STRING_BUFFER_SIZE, "%02d,", pNavData->numSatellitesCalcPos);
  Gga += StringBuffer;

  /* Set the HDOP to string. */
  snprintf(StringBuffer, STRING_BUFFER_SIZE, ",");
  if (pNavData->posDataExist)
  {
    /* Position fixed. */
    if (pNavData->hdop != -1.0) {
      snprintf(StringBuffer, STRING_BUFFER_SIZE, "%.1f,", pNavData->hdop);
    }
  }
  Gga += StringBuffer;

  /* Set the MSL altitude. */
  /* Set the MSL units. */
  snprintf(StringBuffer, STRING_BUFFER_SIZE, ",,");
  if (pNavData->posDataExist)
  {
    /* Position fixed. */
    snprintf(StringBuffer, STRING_BUFFER_SIZE, "%.1f,M,", pNavData->altitude);
  }
  Gga += StringBuffer;

  /* Set the Geiod separation. */
  /* Setthe Geiod separation. */
  snprintf(StringBuffer, STRING_BUFFER_SIZE, ",,");
  if (pNavData->posDataExist)
  {
    /* Skip Geoid */
    snprintf(StringBuffer, STRING_BUFFER_SIZE, ",M,");
  }
  Gga += StringBuffer;

  /* Set the Age of Differential GPS data. Not really applicable. */
  Gga += ",";

  /* Set checksum "*hh". */
  CheckSum = CalcCheckSum(Gga.c_str());
  snprintf(StringBuffer, STRING_BUFFER_SIZE, "*%02X\r\n", CheckSum);
  Gga += StringBuffer;

  return Gga;
}

