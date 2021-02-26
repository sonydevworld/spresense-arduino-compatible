/*
 *  LteScanNetworks.ino - Example for obtaining network information
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
 *
 *  This sketch turns on the modem and gets the following information:
 *  - IP address assigned by LTE network
 *  - Connected LTE network carrier
 *  - Signal strength of the LTE network
 *
 */
 
// libraries
#include <LTE.h>

// APN name
#define APP_LTE_APN "apn" // replace your APN

/* APN authentication settings
 * Ignore these parameters when setting LTE_NET_AUTHTYPE_NONE.
 */
#define APP_LTE_USER_NAME "user"     // replace with your username
#define APP_LTE_PASSWORD  "password" // replace with your password

// APN IP type
#define APP_LTE_IP_TYPE (LTE_NET_IPTYPE_V4V6) // IP : IPv4v6
// #define APP_LTE_IP_TYPE (LTE_NET_IPTYPE_V4) // IP : IPv4
// #define APP_LTE_IP_TYPE (LTE_NET_IPTYPE_V6) // IP : IPv6

// APN authentication type
#define APP_LTE_AUTH_TYPE (LTE_NET_AUTHTYPE_CHAP) // Authentication : CHAP
// #define APP_LTE_AUTH_TYPE (LTE_NET_AUTHTYPE_PAP) // Authentication : PAP
// #define APP_LTE_AUTH_TYPE (LTE_NET_AUTHTYPE_NONE) // Authentication : NONE

/* RAT to use
 * Refer to the cellular carriers information
 * to find out which RAT your SIM supports.
 * The RAT set on the modem can be checked with LTEModemVerification::getRAT().
 */

#define APP_LTE_RAT (LTE_NET_RAT_CATM) // RAT : Cat.M
// #define APP_LTE_RAT (LTE_NET_RAT_NBIOT) // RAT : NB-IoT

// initialize the library instance
LTE        lteAccess;
LTEScanner scannerNetworks;

void setup() {
  // initialize serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  Serial.println("LTE networks scanner");

  /* Power on the modem and Enable the radio function. */

  if (lteAccess.begin() != LTE_SEARCHING) {
    Serial.println("Could not transition to LTE_SEARCHING.");
    Serial.println("Please check the status of the LTE board.");
    for (;;) {
      sleep(1);
    }
  }

  while (true) {
    /* The connection process to the APN will start.
     * If the synchronous parameter is false,
     * the return value will be returned when the connection process is started.
     */
    if (lteAccess.attach(APP_LTE_RAT,
                         APP_LTE_APN,
                         APP_LTE_USER_NAME,
                         APP_LTE_PASSWORD,
                         APP_LTE_AUTH_TYPE,
                         APP_LTE_IP_TYPE) == LTE_READY) {
      Serial.println("attach succeeded.");
      break;
    }

    /* If the following logs occur frequently, one of the following might be a cause:
     * - APN settings are incorrect
     * - SIM is not inserted correctly
     * - If you have specified LTE_NET_RAT_NBIOT for APP_LTE_RAT,
     *   your LTE board may not support it.
     */
    Serial.println("An error has occurred. Retry the network attach process after 1 second.");
    sleep(1);
  }
}

void loop() {

  // Wait 1 second to get the IP address correctly.
  sleep(1);

   // Assigned IP address
  IPAddress address = lteAccess.getIPAddress();
  Serial.print("IP address: ");
  Serial.println(address);

  // currently connected carrier
  Serial.print("Current carrier: ");
  Serial.println(scannerNetworks.getCurrentCarrier());

  // return signal strength
  Serial.print("Signal Strength: ");
  Serial.print(scannerNetworks.getSignalStrength());
  Serial.println(" [dBm]");
}
