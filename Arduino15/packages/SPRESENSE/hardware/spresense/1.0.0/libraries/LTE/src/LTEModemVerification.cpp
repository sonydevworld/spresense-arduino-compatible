/*
 *  LTEModemVerification.cpp - LTEModemVerification implementation file for Spresense Arduino
 *  Copyright 2019 Sony Semiconductor Solutions Corporation
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
  int32_t result = 0;
  int8_t imei[LTE_IMEI_LEN] = {0};

  result = lte_get_imei_sync(imei);
  if (result < 0) {
    LTEERR("lte_get_imei_sync result error : %d\n", result);
    if (-EPROTO == result) {
      theLTECore.printErrorInfo();
    }
    return String("N/A");
  }

  LTEDBG("Successful get IMEI : %s\n", imei);

  return String(reinterpret_cast<char*>(imei));
}

String LTEModemVerification::getFirmwareVersion()
{
  int32_t       result = 0;
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

  return String(reinterpret_cast<char*>(fwVersion.np_package));
}

LTEModemStatus LTEModemVerification::getStatus()
{
  return theLTECore.getStatus();
}
