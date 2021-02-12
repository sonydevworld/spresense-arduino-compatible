/*
 *  LteGnssTracker.ino - Example for publish location using MQTT
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
 *  This example publishes the location acquired with GNSS to the broker using MQTT.
 */

// libraries
#include <RTC.h>
#include <SDHCI.h>
#include <GNSS.h>
#include <LTE.h>
#include <ArduinoMqttClient.h>
#include "gnss_nmea.h"

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

// MQTT broker
#define BROKER_NAME        "your-MQTT-broker" // replace with your broker
#define BROKER_PORT        8883               // port 8883 is the default for MQTT over TLS.
#define ROOTCA_FILE "path/to/cafile"   // Define the path to a file containing CA
                                       // certificates that are trusted.
#define CERT_FILE   "path/to/certfile" // Define the path to a file containing certificate
                                       // for this client, if required by the server.
#define KEY_FILE    "path/to/keyfile"  // Define the path to a file containing private key
                                       // for this client, if required by the server.

// MQTT topic
#define MQTT_TOPIC         "spresense/gnss_tracker" // replace with your topic

// MQTT publish interval settings
#define PUBLISH_INTERVAL_SEC   1   // MQTT publish interval in sec
#define MAX_NUMBER_OF_PUBLISH  60  // Maximum number of publish

LTE lteAccess;
LTETLSClient client;
MqttClient mqttClient(client);
SDClass theSD;
SpGnss Gnss;

int numOfPubs = 0;
unsigned long lastPubSec = 0;
char broker[] = BROKER_NAME;
int port = BROKER_PORT;
char topic[]  = MQTT_TOPIC;

void printClock(RtcTime &rtc)
{ 
  printf("%04d/%02d/%02d %02d:%02d:%02d\n",
         rtc.year(), rtc.month(), rtc.day(),
         rtc.hour(), rtc.minute(), rtc.second());
}

void setup()
{
  // Open serial communications and wait for port to open
  Serial.begin(115200);
  while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Starting GNSS tracker via LTE.");

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
                         APP_LTE_IP_TYPE,
                         false) == LTE_CONNECTING) {
      Serial.println("Attempting to connect to network.");
      break;
    }

    /* If the following logs occur frequently, one of the following might be a cause:
     * - APN settings are incorrect
     * - SIM is not inserted correctly
     * - If you have specified LTE_NET_RAT_NBIOT for APP_LTE_RAT,
     *   your LTE board may not support it.
     */
    Serial.println("An error has occurred. Retry the network attach preparation process after 1 second.");
    sleep(1);
  }

  int result;

  /* Activate GNSS device */
  result = Gnss.begin();
  assert(result == 0);

  /* Start positioning */
  result = Gnss.start();
  assert(result == 0);
  Serial.println("Gnss setup OK");

  // Wait for the modem to connect to the LTE network.
  Serial.println("Waiting for successful attach.");
  while(LTE_READY != lteAccess.getStatus()) {
    sleep(1);
  }
  Serial.println("attach succeeded.");

  // Set local time (not UTC) obtained from the network to RTC.
  RTC.begin();
  unsigned long currentTime;
  while(0 == (currentTime = lteAccess.getTime())) {
    sleep(1);
  }
  RtcTime rtc(currentTime);
  printClock(rtc);
  RTC.setTime(rtc);

  // Set certifications via a file on the SD card before connecting to the MQTT broker
  File rootCertsFile = theSD.open(ROOTCA_FILE, FILE_READ);
  client.setCACert(rootCertsFile, rootCertsFile.available());
  rootCertsFile.close();

  File certsFile = theSD.open(CERT_FILE, FILE_READ);
  client.setCertificate(certsFile, certsFile.available());
  certsFile.close();

  File priKeyFile = theSD.open(KEY_FILE, FILE_READ);
  client.setPrivateKey(priKeyFile, priKeyFile.available());
  priKeyFile.close();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    // do nothing forevermore:
    for (;;)
      sleep(1);
  }

  Serial.println("You're connected to the MQTT broker!");
}

void loop()
{
  /* Check update. */
  if (Gnss.waitUpdate(-1)) {
    /* Get navData. */
    SpNavData navData;
    Gnss.getNavData(&navData);

    bool posFix = ((navData.posDataExist) && (navData.posFixMode != FixInvalid));
    if (posFix) {
      Serial.println("Position is fixed.");
      String nmeaString = getNmeaGga(&navData);
      if (strlen(nmeaString.c_str()) != 0) {
        unsigned long currentTime = lteAccess.getTime();
        if (currentTime >= lastPubSec + PUBLISH_INTERVAL_SEC) {
          // Publish to broker
          Serial.print("Sending message to topic: ");
          Serial.println(topic);
          Serial.print("Publish: ");
          Serial.println(nmeaString);

          // send message, the Print interface can be used to set the message contents
          mqttClient.beginMessage(topic);
          mqttClient.print(nmeaString);
          mqttClient.endMessage();
          lastPubSec = currentTime;
          numOfPubs++;
        }
      }
    } else {
      Serial.println("Position is not fixed.");
    }
  }

  if (numOfPubs >= MAX_NUMBER_OF_PUBLISH) {
    Serial.println("Publish end");
    // do nothing forevermore:
    for (;;)
      sleep(1);
  }
}
