/*
 *  gnss.ino - GNSS example application
 *  Copyright 2018 Sony Semiconductor Solutions Corporation
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
 * @file gnss.ino
 * @author Sony Corporation
 * @brief GNSS example application
 * @details Spresense has an built in GNSS receiver which supports GPS and other
 *          GNSS satellites. This skecth provides an example the GNSS operation.
 *          Simply upload the sketch, reset the board and check the USB serial 
 *          output. After 3 seconds status information should start to appear.\n\n
 *
 *          This example code is in the public domain.
 */

/* include the GNSS library */
#include <GNSS.h>

#define STRING_BUFFER_SIZE  128       /**< %Buffer size */

#define RESTART_CYCLE       (60 * 5)  /**< positioning test term */

static SpGnss Gnss;                   /**< SpGnss object */

/**
 * @brief Turn on / off the LED0 for CPU active notification.
 */
static void Led_isActive(void)
{
  static int state = 1;
  if (state == 1)
  {
    ledOn(PIN_LED0);
    state = 0;
  }
  else
  {
    ledOff(PIN_LED0);
    state = 1;
  }
}

/**
 * @brief Turn on / off the LED1 for positioning state notification.
 * 
 * @param [in] state Positioning state
 */
static void Led_isPosfix(bool state)
{
  if (state)
  {
    ledOn(PIN_LED1);
  }
  else
  {
    ledOff(PIN_LED1);
  }
}

/**
 * @brief Turn on / off the LED3 for error notification.
 * 
 * @param [in] state Error state
 */
static void Led_isError(bool state)
{
  if (state)
  {
    ledOn(PIN_LED3);
  }
  else
  {
    ledOff(PIN_LED3);
  }
}

/**
 * @brief Activate GNSS device and start positioning.
 */
void setup() {
  /* put your setup code here, to run once: */

  int error_flag = 0;

  /* Set serial baudrate. */
  Serial.begin(115200);

  /* Wait HW initialization done. */
  sleep(3);

  /* Turn on all LED:Setup start. */
  ledOn(PIN_LED0);
  ledOn(PIN_LED1);
  ledOn(PIN_LED2);
  ledOn(PIN_LED3);

  /* Set Debug mode to Info */
  Gnss.setDebugMode(PrintInfo);

  int result;

  /* Activate GNSS device */
  result = Gnss.begin();

  if (result != 0)
  {
    Serial.println("Gnss begin error!!");
    error_flag = 1;
  }
  else
  {
    /* Start positioning */
    result = Gnss.start(COLD_START);
    if (result != 0)
    {
      Serial.println("Gnss start error!!");
      error_flag = 1;
    }
    else
    {
      Serial.println("Gnss setup OK");
    }
  }

  /* Turn off all LED:Setup done. */
  ledOff(PIN_LED0);
  ledOff(PIN_LED1);
  ledOff(PIN_LED2);
  ledOff(PIN_LED3);

  /* Set error LED. */
  if (error_flag == 1)
  {
    Led_isError(true);
    exit(0);
  }
}

/**
 * @brief %Print position information.
 */
static void print_pos(SpNavData *pNavData)
{
  char StringBuffer[STRING_BUFFER_SIZE];

  /* print time */
  snprintf(StringBuffer, STRING_BUFFER_SIZE, "%04d/%02d/%02d ", pNavData->time.year, pNavData->time.month, pNavData->time.day);
  Serial.print(StringBuffer);

  snprintf(StringBuffer, STRING_BUFFER_SIZE, "%02d:%02d:%02d.%06d, ", pNavData->time.hour, pNavData->time.minute, pNavData->time.sec, pNavData->time.usec);
  Serial.print(StringBuffer);

  /* print satellites count */
  snprintf(StringBuffer, STRING_BUFFER_SIZE, "numSat:%2d, ", pNavData->numSatellites);
  Serial.print(StringBuffer);

  /* print position data */
  if (pNavData->posDataExist == 0)
  {
    Serial.print("pos not fixed");
  }
  else
  {
    Serial.print("Lat=");
    Serial.print(pNavData->latitude, 6);
    Serial.print(", Lon=");
    Serial.print(pNavData->longitude, 6);
  }

  Serial.println("");
}

/**
 * @brief %Print satellite condition.
 */
static void print_condition(SpNavData *pNavData)
{
  char StringBuffer[STRING_BUFFER_SIZE];
  unsigned long cnt;

  /* Print satellite count. */
  snprintf(StringBuffer, STRING_BUFFER_SIZE, "numSatellites:%2d\n", pNavData->numSatellites);
  Serial.print(StringBuffer);

  for (cnt = 0; cnt < pNavData->numSatellites; cnt++)
  {
    const char *pType = "---";

    /* Get satellite type. */
    /* Keep it to three letters. */
    if ( pNavData->isSatelliteTypeGps(cnt) )
    {
      pType = "GPS";
    }
    else if ( pNavData->isSatelliteTypeGlonass(cnt))
    {
      pType = "GLN";
    }

    /* Get print conditions. */
    unsigned long Id  = pNavData->getSatelliteId(cnt);
    unsigned long Elv = pNavData->getSatelliteElevation(cnt);
    unsigned long Azm = pNavData->getSatelliteAzimuth(cnt);
    float sigLevel = pNavData->getSatelliteSignalLevel(cnt);

    /* Print satellite condition. */
    snprintf(StringBuffer, STRING_BUFFER_SIZE, "[%2d] Type:%s, Id:%2d, Elv:%2d, Azm:%3d, Lv:", cnt, pType, Id, Elv, Azm );
    Serial.print(StringBuffer);
    Serial.println(sigLevel, 6);
  }
}

/**
 * @brief %Print position information and satellite condition.
 * 
 * @details When the loop count reaches the RESTART_CYCLE value, GNSS device is 
 *          restarted.
 */
void loop()
{
  /* put your main code here, to run repeatedly: */

  static int LoopCount = 0;
  static int LastPrintMin = 0;

  /* Blink LED. */
  Led_isActive();

  /* Check update. */
  if (Gnss.waitUpdate(-1))
  {
    /* Get NaviData. */
    SpNavData NavData;
    Gnss.getNavData(&NavData);

    /* Set posfix LED. */
    bool LedSet = (NavData.posDataExist && (NavData.posFixMode != FixInvalid));
    Led_isPosfix(LedSet);

    /* Print satellite information every minute. */
    if (NavData.time.minute != LastPrintMin)
    {
      print_condition(&NavData);
      LastPrintMin = NavData.time.minute;
    }

    /* Print position information. */
    print_pos(&NavData);
  }
  else
  {
    /* Not update. */
    Serial.println("data not update");
  }

  /* Check loop count. */
  LoopCount++;
  if (LoopCount >= RESTART_CYCLE)
  {
    int error_flag = 0;

    /* Turn off LED0 */
    ledOff(PIN_LED0);

    /* Set posfix LED. */
    Led_isPosfix(false);

    /* Restart GNSS. */
    if (Gnss.stop() != 0)
    {
      Serial.println("Gnss stop error!!");
      error_flag = 1;
    }
    else if (Gnss.end() != 0)
    {
      Serial.println("Gnss end error!!");
      error_flag = 1;
    }
    else
    {
      Serial.println("Gnss stop OK.");
    }

    if (Gnss.begin() != 0)
    {
      Serial.println("Gnss begin error!!");
      error_flag = 1;
    }
    else if (Gnss.start(HOT_START) != 0)
    {
      Serial.println("Gnss start error!!");
      error_flag = 1;
    }
    else
    {
      Serial.println("Gnss restart OK.");
    }

    LoopCount = 0;

    /* Set error LED. */
    if (error_flag == 1)
    {
      Led_isError(true);
      exit(0);
    }
  }
}
