/*
 *  LteHttpSecureClient.ino - Example for secure HTTP client using LTE
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
 *  This sketch connects to a website via LTE.
 *  In this example, an HTTP GET request is sent to https://httpin.org/get, 
 *  and an HTTP POST request is sent to https://httpin.org/post. 
 */

// libraries
#include <ArduinoHttpClient.h>
#include <RTC.h>
#include <SDHCI.h>
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

// URL, path & port (for example: httpbin.org)
char server[] = "httpbin.org";
char getPath[] = "/get";
char postPath[] = "/post";
int port = 443; // port 443 is the default for HTTPS

#define ROOTCA_FILE "path/to/cafile"   // Define the path to a file containing CA 
                                       // certificates that are trusted.
#define CERT_FILE   "path/to/certfile" // Define the path to a file containing certificate
                                       // for this client, if required by the server.
#define KEY_FILE    "path/to/keyfile"  // Define the path to a file containing private key
                                       // for this client, if required by the server.

// initialize the library instance
LTE lteAccess;
LTETLSClient tlsClient;
HttpClient client = HttpClient(tlsClient, server, port);
SDClass theSD;

void printClock(RtcTime &rtc)
{ 
  printf("%04d/%02d/%02d %02d:%02d:%02d\n",
         rtc.year(), rtc.month(), rtc.day(),
         rtc.hour(), rtc.minute(), rtc.second());
}

void setup()
{
  // initialize serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Starting secure HTTP client.");

  /* Initialize SD */
  while (!theSD.begin()) {
    ; /* wait until SD card is mounted. */
  }

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

  // Set local time (not UTC) obtained from the network to RTC.
  RTC.begin();
  unsigned long currentTime;
  while(0 == (currentTime = lteAccess.getTime())) {
    sleep(1);
  }
  RtcTime rtc(currentTime);
  printClock(rtc);
  RTC.setTime(rtc);
}

void loop()
{
  // Set certifications via a file on the SD card before connecting to the server
  File rootCertsFile = theSD.open(ROOTCA_FILE, FILE_READ);
  tlsClient.setCACert(rootCertsFile, rootCertsFile.available());
  rootCertsFile.close();
  
  // Remove these commented out if client authentication is required.
  //File certsFile = theSD.open(CERT_FILE, FILE_READ);
  //tlsClient.setCertificate(certsFile, certsFile.available());
  //certsFile.close();
  //File priKeyFile = theSD.open(KEY_FILE, FILE_READ);
  //tlsClient.setPrivateKey(priKeyFile, priKeyFile.available());
  //priKeyFile.close();

  // HTTP GET method
  Serial.println("making GET request");
  client.get(getPath);

  // read the status code and body of the response
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
  Serial.println("Wait five seconds");
  sleep(5);

  // HTTP POST method
  Serial.println("making POST request");
    
  String contentType = "application/x-www-form-urlencoded";
  String postData = "name=Alice&age=12";

  client.post(postPath, contentType, postData);

  // read the status code and body of the response
  statusCode = client.responseStatusCode();
  response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
  
  // do nothing forevermore:
  for (;;)
    sleep(1);
}
