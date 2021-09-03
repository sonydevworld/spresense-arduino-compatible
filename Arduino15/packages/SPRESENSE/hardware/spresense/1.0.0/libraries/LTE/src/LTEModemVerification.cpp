/*
 *  LTEModemVerification.cpp - LTEModemVerification implementation file for Spresense Arduino
 *  Copyright 2019, 2021 Sony Semiconductor Solutions Corporation
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
 * @file LTEModemVerification.cpp
 * @author Sony Semiconductor Solutions Corporation
 *
 * @brief LTE Library for Spresense Arduino.
 *
 * @details LTE modem information management class for Spresense Arduino.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <errno.h>
#include <Print.h>

#include <lte/lte_api.h>
#include <LTECore.h>
#include <LTEModemVerification.h>

/****************************************************************************
 * LTEModemVerification implementation.
 ****************************************************************************/

LTEModemVerification::LTEModemVerification()
{
}

LTEModemVerification::~LTEModemVerification()
{
}

LTEModemStatus LTEModemVerification::begin()
{
  return theLTECore.begin(true);
}

String LTEModemVerification::getIMEI()
{
  int result = 0;
  char imei[LTE_IMEI_LEN] = {0};

  result = lte_get_imei_sync(imei, LTE_IMEI_LEN);
  if (result < 0) {
    LTEERR("lte_get_imei_sync result error : %d\n", result);
    if (-EPROTO == result) {
      theLTECore.printErrorInfo();
    }
    return String("N/A");
  }

  LTEDBG("Successful get IMEI : %s\n", imei);

  return String(imei);
}

String LTEModemVerification::getFirmwareVersion()
{
  int           result = 0;
  lte_version_t fwVersion;
  
  memset(&fwVersion, 0, sizeof(lte_version_t));

  result = lte_get_version_sync(&fwVersion);
  if (result < 0) {
    LTEERR("lte_get_version_sync result error : %d\n", result);
    if (-EPROTO == result) {
      theLTECore.printErrorInfo();
    }
    return String("N/A");
  }

  LTEDBG("Successful get version : %s\n", fwVersion.np_package);

  return String(fwVersion.np_package);
}

LTENetworkRatType LTEModemVerification::getRAT()
{
  int result = 0;

  result = lte_get_rat_sync();
  if (result < 0) {
    if (result == -ENOTSUP) {
      LTEDBG("This API is not supported by the FW version of your modem.\n");
      LTEDBG("Returns LTE_NET_RAT_CATM.\n");
      return LTE_NET_RAT_CATM;
    } else {
      LTEERR("lte_get_rat_sync result error : %d\n", result);
      return LTE_NET_RAT_UNKNOWN;
    }
  } else {
    LTEDBG("Successful get RAT : %d\n", result);
  }
  return (LTENetworkRatType)result;
}

LTEModemStatus LTEModemVerification::getStatus()
{
  return theLTECore.getStatus();
}
