/*
 *  LTEAccessProvider.cpp - LTEAccessProvider implementation file for Spresense Arduino
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
 * @file LTEAccessProvider.cpp
 *
 * @author Sony Semiconductor Solutions Corporation
 *
 * @brief LTE Library for Spresense Arduino.
 *
 * @details LTE connection management class for SPRESENSE LTE library.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <Print.h>
#include <stdint.h>
#include <errno.h>
#include <IPAddress.h>

#include <lte/lte_api.h>
#include <LTECore.h>
#include <LTEAccessProvider.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define LTE_NET_PDN_NUM (2)

/****************************************************************************
 * LTEAccessProvider implementation.
 ****************************************************************************/

LTEAccessProvider::LTEAccessProvider()
{
}

LTEAccessProvider::~LTEAccessProvider()
{
}

LTEModemStatus LTEAccessProvider::begin(char* pinCode, bool restart, bool synchronous)
{
  LTEModemStatus networkStatus = LTE_OFF;

  networkStatus = theLTECore.begin(restart);
  if (LTE_ERROR != networkStatus) {
    networkStatus = theLTECore.startSearchNetwork(pinCode, synchronous);
    theLTECore.setStatus(networkStatus);
  }
  return networkStatus;
}

void LTEAccessProvider::shutdown()
{
  theLTECore.shutdown();
}

LTEModemStatus LTEAccessProvider::attach(const char *apn,
                                         const char *userName,
                                         const char *password,
                                         LTENetworkAuthType authType,
                                         LTENetworkIPType ipType,
                                         bool synchronous)
{
  LTEModemStatus networkStatus = getStatus();
  if (LTE_CONNECTING == networkStatus) {
    LTEERR("This method cannot be called while waiting for a connection.\n");
    theLTECore.setStatus(LTE_ERROR);
    return LTE_ERROR;
  }

  networkStatus = theLTECore.connectNetwork(apn, userName, password, authType, ipType, synchronous);
  theLTECore.setStatus(networkStatus);

  return networkStatus;
}

LTEModemStatus LTEAccessProvider::attach(LTENetworkRatType rat,
                                         const char *apn,
                                         const char *userName,
                                         const char *password,
                                         LTENetworkAuthType authType,
                                         LTENetworkIPType ipType,
                                         bool synchronous)
{
  int            result        = 0;
  LTEModemStatus networkStatus = getStatus();
  if (LTE_CONNECTING == networkStatus) {
    LTEERR("This method cannot be called while waiting for a connection.\n");
    theLTECore.setStatus(LTE_ERROR);
    return LTE_ERROR;
  }

  result = lte_set_rat_sync((uint8_t)rat, true);
  if (result < 0) {
    if (result == -ENOTSUP) {
      if (LTE_NET_RAT_CATM != rat) {
        LTEERR("RAT changes are not supported in the FW version of the modem.\n");
        theLTECore.setStatus(LTE_ERROR);
        return LTE_ERROR;
      } else {
        LTEDBG("RAT changes are not supported in the FW version of the modem.\n");
        LTEDBG("LTE_NET_RAT_CATM is already set on the modem.\n");
      }
    } else {
      LTEERR("lte_set_rat_sync result error : %d\n", result);
      theLTECore.setStatus(LTE_ERROR);
      return LTE_ERROR;
    }
  } else {
    LTEDBG("Successful set RAT : %d\n", result);
  }

  networkStatus = theLTECore.connectNetwork(apn, userName, password, authType, ipType, synchronous);
  theLTECore.setStatus(networkStatus);

  return networkStatus;
}

LTEModemStatus LTEAccessProvider::detach()
{
  LTEModemStatus networkStatus;

  networkStatus = theLTECore.disconnectNetwork();
  theLTECore.setStatus(networkStatus);

  return networkStatus;
}

IPAddress LTEAccessProvider::getIPAddress()
{
  int            result  = 0;
  lte_netinfo_t  netinfo;
  lte_pdn_t      pdnStatus[LTE_NET_PDN_NUM];
  IPAddress      ipAddress;

  netinfo.pdn_stat = pdnStatus;

  result = lte_get_netinfo_sync(LTE_NET_PDN_NUM, &netinfo);
  if (result < 0) {
    LTEERR("lte_get_netinfo_sync result error : %d\n", result);
    if (-EPROTO == result) {
      theLTECore.printErrorInfo();
    }
    goto exit;
  }

  int i;
  int pdnNo;
  int pdnCount;
  pdnCount = ((LTE_NET_PDN_NUM < netinfo.pdn_num) ? LTE_NET_PDN_NUM : netinfo.pdn_num);
  pdnNo = -1;

  for (i = 0; i < pdnCount; i++) {
    if (0 != (netinfo.pdn_stat[i].apn_type & LTE_APN_TYPE_DEFAULT)) {
      if (netinfo.pdn_stat[i].address[0].ip_type != LTE_IPTYPE_V4) {
        LTEERR("This method does not support formats other than IPv4.\n");
        LTEERR("Assigned IP address : %s\n", netinfo.pdn_stat[i].address[0].address);
        goto exit;
      } else {
        pdnNo = i;
        break;
      }
    }
  }

  if (pdnNo < 0) {
    LTEERR("PDN information could not be obtained.\n");
    LTEERR("nw_stat : %d\n", netinfo.nw_stat);
    goto exit;
  }

  LTEDBG("Successful get IP address : %s\n", netinfo.pdn_stat[pdnNo].address[0].address);

  if (!ipAddress.fromString(netinfo.pdn_stat[pdnNo].address[0].address)) {
    LTEERR("IP address converting error.\n");
  }

exit:
  return ipAddress;
}

unsigned long LTEAccessProvider::getTime()
{
  int result = 0;

  if (LTE_READY != getStatus()) {
    LTEERR("Cannot be called with the current status. : %d\n", getStatus());
    return 0;
  }

  lte_localtime_t localTime;

  result = lte_get_localtime_sync(&localTime);
  if (result < 0) {  
    LTEERR("lte_get_localtime_sync result error : %d\n", result);
    if (-EPROTO == result) {
      theLTECore.printErrorInfo();
    }
    return 0;
  }

  LTEDBG("Successful get localtime : %4ld/%02ld/%02ld,%02ld:%02ld:%02ld\n",
         localTime.year + 1900 + 100, localTime.mon, localTime.mday,
         localTime.hour, localTime.min, localTime.sec);

  struct tm calTime;
  memset(&calTime, 0, sizeof(struct tm));

  calTime.tm_year = localTime.year + 100; /* 1900 + 100 + year(0-99) */
  calTime.tm_mon  = localTime.mon - 1;    /* mon(1-12) - 1 */
  calTime.tm_mday = localTime.mday;
  calTime.tm_hour = localTime.hour;
  calTime.tm_min  = localTime.min;
  calTime.tm_sec  = localTime.sec;

  unsigned long timeSecond = mktime(&calTime);

  if (timeSecond == (time_t)-1) {
    LTEERR("mktime error : 0x%lX\n", timeSecond);
    return 0;
  } else {
    return timeSecond;
  }
}

LTEModemStatus LTEAccessProvider::getStatus()
{
  return theLTECore.getStatus();
}
