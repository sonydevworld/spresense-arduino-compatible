/*
 *  LteHttpSecureClient.ino - Example for secure HTTP client using LTE
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

// APN data
#define LTE_APN       "apn"      // replace your APN
#define LTE_USER_NAME "user"     // replace with your username
#define LTE_PASSWORD  "password" // replace with your password

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

  // If your SIM has PIN, pass it as a parameter of begin() in quotes
  while (true) {
    if (lteAccess.begin() == LTE_SEARCHING) {
      if (lteAccess.attach(LTE_APN, LTE_USER_NAME, LTE_PASSWORD) == LTE_READY) {
        Serial.println("attach succeeded.");
        break;
      }
      Serial.println("An error occurred, shutdown and try again.");
      lteAccess.shutdown();
      sleep(1);
    }
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
