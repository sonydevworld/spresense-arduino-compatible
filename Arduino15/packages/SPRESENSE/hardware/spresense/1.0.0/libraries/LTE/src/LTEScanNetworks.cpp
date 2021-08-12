/*
 *  LTEScanNetworks.cpp - LTEScanNetworks implementation file for Spresense Arduino
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
 * @file LTEScanNetworks.cpp
 *
 * @author Sony Semiconductor Solutions Corporation
 *
 * @brief LTE Library for Spresense Arduino.
 *
 * @details LTE network information management class for Spresense Arduino.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <errno.h>
#include <Print.h>

#include <lte/lte_api.h>
#include <LTECore.h>
#include <LTEScanNetworks.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define LTE_SCANNER_ERR_STR "N/A"

/****************************************************************************
 * LTEScanNetworks implementation.
 ****************************************************************************/

LTEScanNetworks::LTEScanNetworks()
{
}

LTEScanNetworks::~LTEScanNetworks()
{
}

LTEModemStatus LTEScanNetworks::begin()
{
  return theLTECore.begin(true);
}

String LTEScanNetworks::getSignalStrength()
{
  int           result  = 0;
  lte_quality_t quality;

  result = lte_get_quality_sync(&quality);
  if (result < 0) {
    LTEERR("lte_get_quality_sync result error : %d\n", result);
    if (-EPROTO == result) {
      theLTECore.printErrorInfo();
    }
    return String("N/A");
  }

  if (LTE_VALID != quality.valid) {
    LTEERR("Invalid quality information.\n");
    return String("N/A");
  } else {
    LTEDBG("Successful get signal quality : %d\n", quality.rssi);
    return String(quality.rssi);
  }
}

String LTEScanNetworks::getCurrentCarrier()
{
  int     result                    = 0;
  char    carrier[LTE_OPERATOR_LEN] = {0};

  result = lte_get_operator_sync(carrier, LTE_OPERATOR_LEN);
  if (result < 0) {
    LTEERR("lte_get_operator_sync result error : %d\n", result);
    if (-EPROTO == result) {
      theLTECore.printErrorInfo();
    }
    return String("N/A");
  }

  if (0 == strlen(carrier)) {
    LTEERR("Carrier name could not be obtained from the LTE network.\n");
    return String("N/A");
  } else {
    LTEDBG("Successful get network carrier : %s\n", carrier);
    return String(carrier);
  }
}

LTEModemStatus LTEScanNetworks::getStatus()
{
  return theLTECore.getStatus();
}
