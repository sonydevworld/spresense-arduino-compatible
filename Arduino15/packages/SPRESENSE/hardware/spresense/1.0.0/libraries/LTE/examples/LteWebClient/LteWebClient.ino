/*
 *  LteWebClient.ino - Example for Web client using LTE
 *  Copyright 2019, 2021, 2022 Sony Semiconductor Solutions Corporation
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
 *  This sketch connects to a website via LTE. Specifically,
 *  this example downloads the URL "http://arduino.tips/asciilogo.txt" and
 *  prints it to the Serial monitor.
 */

// libraries
#include <LTE.h>

// APN name
#define APP_LTE_APN "" // replace your APN

/* APN authentication settings
 * Ignore these parameters when setting LTE_NET_AUTHTYPE_NONE.
 */
#define APP_LTE_USER_NAME "" // replace with your username
#define APP_LTE_PASSWORD  "" // replace with your password

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

#define APP_LTE_RAT (LTE_NET_RAT_CATM) // RAT : LTE-M (LTE Cat-M1)
// #define APP_LTE_RAT (LTE_NET_RAT_NBIOT) // RAT : NB-IoT

// initialize the library instance
LTE lteAccess;
LTEClient client;

// URL, path & port (for example: arduino.cc)
char server[] = "arduino.tips";
char path[] = "/asciilogo.txt";
int port = 80; // port 80 is the default for HTTP

String readFromSerial() {
  /* Read String from serial monitor */
  String str;
  int  read_byte = 0;
  while (true) {
    if (Serial.available() > 0) {
      read_byte = Serial.read();
      if (read_byte == '\n' || read_byte == '\r') {
        Serial.println("");
        break;
      }
      Serial.print((char)read_byte);
      str += (char)read_byte;
    }
  }
  return str;
}

void readApnInformation(char apn[], LTENetworkAuthType *authtype,
                       char user_name[], char password[]) {
  /* Set APN parameter to arguments from readFromSerial() */

  String read_buf;

  while (strlen(apn) == 0) {
    Serial.print("Enter Access Point Name:");
    readFromSerial().toCharArray(apn, LTE_NET_APN_MAXLEN);
  }

  while (true) {
    Serial.print("Enter APN authentication type(CHAP, PAP, NONE):");
    read_buf = readFromSerial();
    if (read_buf.equals("NONE") == true) {
      *authtype = LTE_NET_AUTHTYPE_NONE;
    } else if (read_buf.equals("PAP") == true) {
      *authtype = LTE_NET_AUTHTYPE_PAP;
    } else if (read_buf.equals("CHAP") == true) {
      *authtype = LTE_NET_AUTHTYPE_CHAP;
    } else {
      /* No match authtype */
      Serial.println("No match authtype. type at CHAP, PAP, NONE.");
      continue;
    }
    break;
  }

  if (*authtype != LTE_NET_AUTHTYPE_NONE) {
    while (strlen(user_name)== 0) {
      Serial.print("Enter username:");
      readFromSerial().toCharArray(user_name, LTE_NET_USER_MAXLEN);
    }
    while (strlen(password) == 0) {
      Serial.print("Enter password:");
      readFromSerial().toCharArray(password, LTE_NET_PASSWORD_MAXLEN);
    }
  }

  return;
}

void setup()
{
  char apn[LTE_NET_APN_MAXLEN] = APP_LTE_APN;
  LTENetworkAuthType authtype = APP_LTE_AUTH_TYPE;
  char user_name[LTE_NET_USER_MAXLEN] = APP_LTE_USER_NAME;
  char password[LTE_NET_PASSWORD_MAXLEN] = APP_LTE_PASSWORD;

  // initialize serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Starting web client.");

  /* Set if Access Point Name is empty */
  if (strlen(APP_LTE_APN) == 0) {
    Serial.println("This sketch doesn't have a APN information.");
    readApnInformation(apn, &authtype, user_name, password);
  }
  Serial.println("=========== APN information ===========");
  Serial.print("Access Point Name  : ");
  Serial.println(apn);
  Serial.print("Authentication Type: ");
  Serial.println(authtype == LTE_NET_AUTHTYPE_CHAP ? "CHAP" :
                 authtype == LTE_NET_AUTHTYPE_NONE ? "NONE" : "PAP");
  if (authtype != LTE_NET_AUTHTYPE_NONE) {
    Serial.print("User Name          : ");
    Serial.println(user_name);
    Serial.print("Password           : ");
    Serial.println(password);
  }

  while (true) {

    /* Power on the modem and Enable the radio function. */

    if (lteAccess.begin() != LTE_SEARCHING) {
      Serial.println("Could not transition to LTE_SEARCHING.");
      Serial.println("Please check the status of the LTE board.");
      for (;;) {
        sleep(1);
      }
    }

    /* The connection process to the APN will start.
     * If the synchronous parameter is false,
     * the return value will be returned when the connection process is started.
     */
    if (lteAccess.attach(APP_LTE_RAT,
                         apn,
                         user_name,
                         password,
                         authtype,
                         APP_LTE_IP_TYPE) == LTE_READY) {
      Serial.println("attach succeeded.");
      break;
    }

    /* If the following logs occur frequently, one of the following might be a cause:
     * - APN settings are incorrect
     * - SIM is not inserted correctly
     * - If you have specified LTE_NET_RAT_NBIOT for APP_LTE_RAT,
     *   your LTE board may not support it.
     * - Rejected from LTE network
     */
    Serial.println("An error has occurred. Shutdown and retry the network attach process after 1 second.");
    lteAccess.shutdown();
    sleep(1);
  }

  // if you get a connection, report back via serial:
  if (client.connect(server, port)) {
    Serial.println("connected");
    // Make a HTTP request:
    client.print("GET ");
    client.print(path);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}

void loop()
{
  // if there are incoming bytes available
  // from the server, read them and print them:
  if (int len = client.available()) {
    char buff[len + 1];
    buff[len] = '\0';
    client.read((uint8_t*)buff, len);
    Serial.print(buff);
  }

  // if the server's disconnected, stop the client:
  if (!client.available() && !client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();

    // do nothing forevermore:
    for (;;)
      sleep(1);
  }
}
