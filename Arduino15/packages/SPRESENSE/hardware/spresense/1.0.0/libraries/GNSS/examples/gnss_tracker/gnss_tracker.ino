/*
 *  gnss_tracker.ino - GNSS tracker example application
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
 * @file gnss_tracker.ino
 * @author Sony Corporation
 * @brief GNSS tracker example application
 * @details The gnss_tracker is a sample sketch that performs GPS positioning by
 *          intermittent operation.
 */

#include <GNSS.h>
#include <GNSSPositionData.h>
#include "gnss_tracker.h"
#include "gnss_nmea.h"
#include "gnss_file.h"

/* Config file */
#define CONFIG_FILE_NAME    "tracker.ini"  /**< Config file name */
#define CONFIG_FILE_SIZE    4096           /**< Config file size */

/* Index file */
#define INDEX_FILE_NAME    "index.ini"     /**< Index file name */
#define INDEX_FILE_SIZE    16              /**< Index file size */

#define STRING_BUFFER_SIZE  128            /**< %String buffer size */
#define NMEA_BUFFER_SIZE    128            /**< NMEA buffer size */
#define OUTPUT_FILENAME_LEN 16             /**< Output file name length */

/* Default parameter. */
#define DEFAULT_INTERVAL_SEC    1          /**< Default positioning interval in seconds*/
#define DEFAULT_ACTIVE_SEC      60         /**< Default positioning active in seconds */
#define DEFAULT_SEEP_SEC        240        /**< Default positioning sleep in seconds */
#define INITIAL_ACTIVE_TIME     300        /**< Initial positioning active in seconds */
#define IDLE_ACTIVE_TIME        600        /**< Idle positioning active in seconds */

#define SERIAL_BAUDRATE     115200         /**< Serial baud rate */

#define SEPARATOR           0x0A           /**< Separator */

/**
 * @enum LoopState
 * @brief State of loop
 */
enum LoopState {
  eStateSleep,  /**< Loop is not activated */
  eStateActive  /**< Loop is activated */
};

/**
 * @enum TrackerMode
 * @brief Tracker mode
 */
enum TrackerMode {
  eModeNormal = 0,  /**< Run positioning. Output NMEA text. */
  eModeShell        /**< Run nsh to print and delete NMEA text. */
};

/**
 * @enum ParamSat
 * @brief Satellite system
 */
enum ParamSat {
  eSatGps,      /**< GPS */
  eSatGlonass,  /**< GLONASS */
  eSatAll       /**< ALL */
};

/**
 * @struct ConfigParam
 * @brief Configuration parameters
 */
typedef struct
{
  ParamSat      SatelliteSystem;  /**< Satellite system(GPS/GLONASS/ALL). */
  boolean       NmeaOutUart;      /**< Output NMEA message to UART(TRUE/FALSE). */
  boolean       NmeaOutFile;      /**< Output NMEA message to file(TRUE/FALSE). */
  boolean       BinaryOut;        /**< Output binary data to file(TRUE/FALSE). */
  unsigned long IntervalSec;      /**< Positioning interval sec(1-300). */
  unsigned long ActiveSec;        /**< Positioning active sec(60-300). */
  unsigned long SleepSec;         /**< Positioning sleep sec(0-240). */
  SpPrintLevel  UartDebugMessage; /**< Uart debug message(NONE/ERROR/WARNING/INFO). */
} ConfigParam;

SpGnss Gnss;                            /**< SpGnss object */
ConfigParam Parameter;                  /**< Configuration parameters */
unsigned int Mode;                      /**< Tracker mode */
char FilenameTxt[OUTPUT_FILENAME_LEN];  /**< Output file name */
char FilenameBin[OUTPUT_FILENAME_LEN];  /**< Output binary file name */
AppPrintLevel AppDebugPrintLevel;       /**< Print level */
unsigned long SetActiveSec;             /**< Positioning active sec(60-300) */

/**
 * @brief Compare parameter.
 * 
 * @param [in] Input Parameter to compare
 * @param [in] Refer Reference parameter
 * @return 0 if equal
 */
static int ParamCompare(const char *Input , const char *Refer)
{
  /* Set argument. */
  String InputStr = Input;
  String ReferStr = Refer;

  /* Convert to upper case. */
  InputStr.toUpperCase();
  ReferStr.toUpperCase();

  /* Compare. */
  return memcmp(InputStr.c_str(), ReferStr.c_str(), strlen(ReferStr.c_str()));
}

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
  if (state == 1)
  {
    ledOn(PIN_LED1);
  }
  else
  {
    ledOff(PIN_LED1);
  }
}

/**
 * @brief Turn on / off the LED2 for file SD access notification.
 * 
 * @param [in] state SD access state
 */
static void Led_isSdAccess(bool state)
{
  if (state == 1)
  {
    ledOn(PIN_LED2);
  }
  else
  {
    ledOff(PIN_LED2);
  }
}

/**
 * @brief Turn on / off the LED3 for error notification.
 * 
 * @param [in] state Error state
 */
static void Led_isError(bool state)
{
  if (state == 1)
  {
    ledOn(PIN_LED3);
  }
  else
  {
    ledOff(PIN_LED3);
  }
}

/**
 * @brief Convert configuration parameters to String
 * 
 * @param [in] pConfigParam Configuration parameters
 * @return Configuration parameters as String
 */
static String MakeParameterString(ConfigParam *pConfigParam)
{
  const char *pComment;
  const char *pParam;
  const char *pData;
  char StringBuffer[STRING_BUFFER_SIZE];
  String ParamString;

  /* Set SatelliteSystem. */
  pComment = "; Satellite system(GPS/GLONASS/ALL)";
  pParam = "SatelliteSystem=";
  switch (pConfigParam->SatelliteSystem)
  {
    case eSatGps:
      pData = "GPS";
      break;

    case eSatGlonass:
      pData = "GLONASS";
      break;

    case eSatAll:
    default:
      pData = "ALL";
      break;
  }
  snprintf(StringBuffer, STRING_BUFFER_SIZE, "%s\n%s%s\n", pComment, pParam, pData);
  ParamString += StringBuffer;

  /* Set NmeaOutUart. */
  pComment = "; Output NMEA message to UART(TRUE/FALSE)";
  pParam = "NmeaOutUart=";
  if (pConfigParam->NmeaOutUart == FALSE)
  {
    pData = "FALSE";
  }
  else
  {
    pData = "TRUE";
  }
  snprintf(StringBuffer, STRING_BUFFER_SIZE, "%s\n%s%s\n", pComment, pParam, pData);
  ParamString += StringBuffer;

  /* Set NmeaOutFile. */
  pComment = "; Output NMEA message to file(TRUE/FALSE)";
  pParam = "NmeaOutFile=";
  if (pConfigParam->NmeaOutFile == FALSE)
  {
    pData = "FALSE";
  }
  else
  {
    pData = "TRUE";
  }
  snprintf(StringBuffer, STRING_BUFFER_SIZE, "%s\n%s%s\n", pComment, pParam, pData);
  ParamString += StringBuffer;

  /* Set BinaryOut. */
  pComment = "; Output binary data to file(TRUE/FALSE)";
  pParam = "BinaryOut=";
  if (pConfigParam->BinaryOut == FALSE)
  {
    pData = "FALSE";
  }
  else
  {
    pData = "TRUE";
  }
  snprintf(StringBuffer, STRING_BUFFER_SIZE, "%s\n%s%s\n", pComment, pParam, pData);
  ParamString += StringBuffer;

  /* Set IntervalSec. */
  pComment = "; Positioning interval sec(1-300)";
  pParam = "IntervalSec=";
  snprintf(StringBuffer, STRING_BUFFER_SIZE, "%s\n%s%d\n", pComment, pParam, pConfigParam->IntervalSec);
  ParamString += StringBuffer;

  /* Set ActiveSec. */
  pComment = "; Positioning active sec(60-300)";
  pParam = "ActiveSec=";
  snprintf(StringBuffer, STRING_BUFFER_SIZE, "%s\n%s%d\n", pComment, pParam, pConfigParam->ActiveSec);
  ParamString += StringBuffer;

  /* Set SleepSec. */
  pComment = "; Positioning sleep sec(0-240)";
  pParam = "SleepSec=";
  snprintf(StringBuffer, STRING_BUFFER_SIZE, "%s\n%s%d\n", pComment, pParam, pConfigParam->SleepSec);
  ParamString += StringBuffer;

  /* Set UartDebugMessage. */
  pComment = "; Uart debug message(NONE/ERROR/WARNING/INFO)";
  pParam = "UartDebugMessage=";
  switch (pConfigParam->UartDebugMessage)
  {
    case PrintError:
      pData = "ERROR";
      break;

    case PrintWarning:
      pData = "WARNING";
      break;

    case PrintInfo:
      pData = "INFO";
      break;

    case PrintNone:
    default:
      pData = "NONE";
      break;
  }
  snprintf(StringBuffer, STRING_BUFFER_SIZE, "%s\n%s%s\n", pComment, pParam, pData);
  ParamString += StringBuffer;

  /* End of file. */
  snprintf(StringBuffer, STRING_BUFFER_SIZE, "; EOF");
  ParamString += StringBuffer;

  return ParamString;
}

/**
 * @brief Read the ini file and set it as a parameter.
 * 
 * @details If there is no description, it will be the default value.
 * @param [out] pConfigParam Configuration parameters
 * @return 0 if success, -1 if failure
 */
static int ReadParameter(ConfigParam *pConfigParam)
{
  char *pReadBuff = NULL;
  pReadBuff = (char*)malloc(CONFIG_FILE_SIZE);
  if (pReadBuff == NULL)
  {
    APP_PRINT_E("alloc error:");
    APP_PRINT_E(CONFIG_FILE_NAME);
    APP_PRINT_E("\n");

    return -1;
  }

  /* Read file. */
  int ReadSize = 0;
  ReadSize = ReadChar(pReadBuff, CONFIG_FILE_SIZE, CONFIG_FILE_NAME, FILE_READ);
  if (ReadSize == 0)
  {
    APP_PRINT_E("read error:");
    APP_PRINT_E(CONFIG_FILE_NAME);
    APP_PRINT_E("\n");

    return -1;
  }

  /* Set NULL at EOF. */
  pReadBuff[ReadSize] = NULL;

  /* Record the start position for each line. */
  int CharCount;
  int LineCount = 0;
  boolean FindSeparator = true;
  char *LineList[128] = {0,};

  for (CharCount = 0; CharCount < ReadSize; CharCount++)
  {
    if (FindSeparator == true)
    {
      LineList[LineCount] = &pReadBuff[CharCount];
      FindSeparator = false;
      LineCount++;
      if (LineCount >= 128)
      {
        break;
      } else {
        /* nop */
      }
    }

    if (pReadBuff[CharCount] == SEPARATOR)
    {
      FindSeparator = true;
      pReadBuff[CharCount] = NULL;
    }
  }

  /* Parse each line. */
  int MaxLine = LineCount;
  char *pParamName;
  char *pParamData;
  int length;
  int tmp;
  for (LineCount = 0; LineCount < MaxLine; LineCount++)
  {
    pParamName = LineList[LineCount];

    pParamData = NULL;
    if (pParamName[0] != ';')
    {
      length = strlen(pParamName);

      for (CharCount = 0; CharCount < length; CharCount++)
      {
        if (pParamName[CharCount] == '=')
        {
          pParamData = &(pParamName[CharCount + 1]);
          break;
        }
      }
    }

    /* Parse start. */
    if (pParamData == NULL)
    {
      /* nop */
    }
    else if (!ParamCompare(pParamName, "SatelliteSystem="))
    {
      if (!ParamCompare(pParamData, "GPS"))
      {
        pConfigParam->SatelliteSystem = eSatGps;
      }
      else if (!ParamCompare(pParamData, "GLONASS"))
      {
        pConfigParam->SatelliteSystem = eSatGlonass;
      }
      else if (!ParamCompare(pParamData, "ALL"))
      {
        pConfigParam->SatelliteSystem = eSatAll;
      }
    }
    else if (!ParamCompare(pParamName, "NmeaOutUart="))
    {
      if (!ParamCompare(pParamData, "FALSE"))
      {
        pConfigParam->NmeaOutUart = false;
      }
      else
      {
        pConfigParam->NmeaOutUart = true;
      }
    }
    else if (!ParamCompare(pParamName, "NmeaOutFile="))
    {
      if (!ParamCompare(pParamData, "FALSE"))
      {
        pConfigParam->NmeaOutFile = false;
      }
      else
      {
        pConfigParam->NmeaOutFile = true;
      }
    }
    else if (!ParamCompare(pParamName, "BinaryOut="))
    {
      if (!ParamCompare(pParamData, "FALSE"))
      {
        pConfigParam->BinaryOut = false;
      }
      else
      {
        pConfigParam->BinaryOut = true;
      }
    }
    else if (!ParamCompare(pParamName, "IntervalSec="))
    {
      tmp = strtoul(pParamData, NULL, 10);
      pConfigParam->IntervalSec = max(1, min(tmp, 300));
    }
    else if (!ParamCompare(pParamName, "ActiveSec="))
    {
      tmp = strtoul(pParamData, NULL, 10);
      pConfigParam->ActiveSec = max(60, min(tmp, 300));
    }
    else if (!ParamCompare(pParamName, "SleepSec="))
    {
      tmp = strtoul(pParamData, NULL, 10);
      pConfigParam->SleepSec = max(0, min(tmp, 240));
    }
    else if (!ParamCompare(pParamName, "UartDebugMessage="))
    {
      if (!ParamCompare(pParamData, "NONE"))
      {
        pConfigParam->UartDebugMessage = PrintNone;
      }
      else if (!ParamCompare(pParamData, "ERROR"))
      {
        pConfigParam->UartDebugMessage = PrintError;
      }
      else if (!ParamCompare(pParamData, "WARNING"))
      {
        pConfigParam->UartDebugMessage = PrintWarning;
      }
      else if (!ParamCompare(pParamData, "INFO"))
      {
        pConfigParam->UartDebugMessage = PrintInfo;
      }
    }
  }

  return OK;
}

/**
 * @brief Create an ini file based on the current parameters.
 * 
 * @param [in] pConfigParam Configuration parameters
 * @return 0 if success, -1 if failure
 */
static int WriteParameter(ConfigParam *pConfigParam)
{
  String ParamString;
  int ret = -1;
  unsigned long write_size;

  /* Make parameter data. */

  ParamString = MakeParameterString(pConfigParam);

  /* Write parameter data. */

  if (strlen(ParamString.c_str()) != 0)
  {
    Led_isSdAccess(true);
    write_size = WriteChar(ParamString.c_str(), CONFIG_FILE_NAME, FILE_WRITE);
    Led_isSdAccess(false);

    if (write_size == strlen(ParamString.c_str()))
    {
      ret = 0;
    }
    else
    {
      Led_isError(true);
    }
  }

  return ret;
}

/**
 * @brief Setup configuration parameters.
 * 
 * @return 0 if success, -1 if failure
 */
static int SetupParameter(void)
{
  int ret;
  String ParamString;

  /* Read parameter file. */
  ret = ReadParameter(&Parameter);
  if (ret != OK)
  {
    /* If there is no parameter file, create a new one. */
    ret = WriteParameter(&Parameter);
  }

  /* Print parameter. */
  ParamString = MakeParameterString(&Parameter);
  APP_PRINT(ParamString.c_str());
  APP_PRINT("\n\n");

  return ret;
}

/**
 * @brief Go to Sleep mode
 */
static void SleepIn(void)
{
  /* Turn off the LED. */
  APP_PRINT("Sleep ");
  ledOff(PIN_LED0);
  Gnss.stop();
  Gnss.end();

  // TBD:Further sleep processing will be implemented.
  APP_PRINT("in.\n");
}

/**
 * @brief Go to Active mode.
 */
static void SleepOut(void)
{
  APP_PRINT("Sleep ");
  // TBD:Further wakeup processing will be implemented.

  Gnss.begin();
  Gnss.start(HOT_START);
  APP_PRINT("out.\n");
}

/**
 * @brief Get file number.
 * 
 * @return File count
 */
unsigned long GetFileNumber(void)
{
  int FileCount;
  char IndexData[INDEX_FILE_SIZE];
  int ReadSize = 0;

  /* Open index file. */
  ReadSize = ReadChar(IndexData, INDEX_FILE_SIZE, INDEX_FILE_NAME, FILE_READ);
  if (ReadSize != 0)
  {
    /* Use index data. */
    FileCount = strtoul(IndexData, NULL, 10);
    FileCount++;

    Remove(INDEX_FILE_NAME);
  }
  else
  {
    /* Init file count. */
    FileCount = 1;
  }

  /* Update index.txt */
  snprintf(IndexData, sizeof(IndexData), "%08d", FileCount);
  WriteChar(IndexData, INDEX_FILE_NAME, FILE_WRITE);

  return FileCount;
}

/**
 * @brief Setup positioning.
 * 
 * @return 0 if success, 1 if failure
 */
static int SetupPositioning(void)
{
  int error_flag = 0;

  /* Set default Parameter. */
  Parameter.SatelliteSystem  = eSatAll;
  Parameter.NmeaOutUart      = true;
  Parameter.NmeaOutFile      = true;
  Parameter.BinaryOut        = false;
  Parameter.IntervalSec      = DEFAULT_INTERVAL_SEC;
  Parameter.ActiveSec        = DEFAULT_ACTIVE_SEC;
  Parameter.SleepSec         = DEFAULT_SEEP_SEC;
  Parameter.UartDebugMessage = PrintNone;

  /* Mount SD card. */
  if (BeginSDCard() != true)
  {
    /* Error case.*/
    APP_PRINT_E("SD begin error!!\n");

    error_flag = 1;
  }
  else
  {
    /* Setup Parameter. */
    SetupParameter();
  }

  /* Set Gnss debug mode. */
  Gnss.setDebugMode(Parameter.UartDebugMessage);
  AppDebugPrintLevel = (AppPrintLevel)Parameter.UartDebugMessage;

  if (Gnss.begin(Serial) != 0)
  {
    /* Error case. */
    APP_PRINT_E("Gnss begin error!!\n");
    error_flag = 1;
  }
  else
  {
    APP_PRINT_I("Gnss begin OK.\n");

    switch (Parameter.SatelliteSystem)
    {
      case eSatGps:
        Gnss.useGps();
        Gnss.unuseGlonass();
        break;

      case eSatGlonass:
        Gnss.unuseGps();
        Gnss.useGlonass();
        break;

      case eSatAll:
      default:
        Gnss.useGps();
        Gnss.useGlonass();
        break;
    }

    Gnss.setInterval(Parameter.IntervalSec);

    if (Gnss.start(HOT_START) != OK)
    {
      /* Error case. */
      APP_PRINT_E("Gnss start error!!\n");
      error_flag = 1;
    }
  }

  /* Create output file name. */
  FilenameTxt[0] = 0;
  FilenameBin[0] = 0;
  if ( (Parameter.NmeaOutFile == true) || (Parameter.BinaryOut == true) )
  {
    int FileCount = GetFileNumber();

    if (Parameter.NmeaOutFile == true)
    {
      /* Create a file name to store NMEA data. */
      snprintf(FilenameTxt, sizeof(FilenameTxt), "%08d.txt", FileCount);
    }
    if (Parameter.BinaryOut == true)
    {
      /* Create a file name to store binary data. */
      snprintf(FilenameBin, sizeof(FilenameBin), "%08d.bin", FileCount);
    }
  }

  return error_flag;
}

/**
 * @brief Activate GNSS device and setup positioning
 */
void setup()
{
  /* put your setup code here, to run once: */

  int error_flag = 0;
  char KeyRead[2] = {0, 0};

  /* Initialize the serial first for debug messages. */
  /* Set serial baudeate. */
  Serial.begin(SERIAL_BAUDRATE);
  APP_PRINT("Please specify the operation mode.\n");
  APP_PRINT("  0:Run positioning. Output NMEA text.\n");
  APP_PRINT("\n");

  /* Wait mode select. */
  sleep(3);

  /* Turn on all LED:Setup start. */
  ledOn(PIN_LED0);
  ledOn(PIN_LED1);
  ledOn(PIN_LED2);
  ledOn(PIN_LED3);

  /* Read key input. */
  KeyRead[0] = Serial.read();

  /* Convet to mode value. */
  Mode = strtoul(KeyRead, NULL, 10);

  APP_PRINT("set mode : ");
  APP_PRINT(Mode);
  APP_PRINT("\n");
  switch (Mode) {
#if 0 // TBD:Not implemented
    case eModeShell:
      /* nop */
      break;
#endif

    case eModeNormal:
    default:
      error_flag = SetupPositioning();
      break;
  }

  SetActiveSec = INITIAL_ACTIVE_TIME;

  /* Turn off all LED:Setup done. */
  ledOff(PIN_LED0);
  ledOff(PIN_LED1);
  ledOff(PIN_LED2);
  ledOff(PIN_LED3);

  /* Set error LED. */
  if (error_flag == 1)
  {
    Led_isError(true);
  }
}

/**
 * @brief GNSS tracker loop
 * 
 * @details Positioning is performed for the first 300 seconds after setup.
 *          After that, in each loop processing, it sleeps for SleepSec 
 *          seconds and performs positioning ActiveSec seconds. 
 *          The gnss_tracker use SatelliteSystem sattelites for positioning.\n\n
 *  
 *          Positioning result is notificated in every IntervalSec second.
 *          The result formatted to NMEA will be saved on SD card if the 
 *          parameter NmeaOutFile is TRUE, or/and output to UART if the 
 *          parameter NmeaOutUart is TRUE. NMEA is buffered for each 
 *          notification. Write at once when ActiveSec completes. If SleepSec 
 *          is set to 0, positioning is performed continuously.
 */
void loop() {
  static int State = eStateActive;
  static int TimeOut = IDLE_ACTIVE_TIME;
  static bool PosFixflag = false;
  static char *pNmeaBuff     = NULL;
  static char *pBinaryBuffer = NULL;

  /* Check state. */
  if (State == eStateSleep)
  {
    /* Sleep. */
    TimeOut--;

    sleep(1);

    APP_PRINT(">");

    /* Counter Check. */
    if (TimeOut <= 0)
    {
      APP_PRINT("\n");

      /* Set active timeout. */
      TimeOut = IDLE_ACTIVE_TIME;
      SetActiveSec = Parameter.ActiveSec;

      /* Set new mode. */
      State = eStateActive;

      /* Go to Active mode. */
      SleepOut();
    }
    else if ((TimeOut % 60) == 0)
    {
      APP_PRINT("\n");
    }
  }
  else
  {
    /* Active. */
    unsigned long BuffSize;
    unsigned long WriteSize;
    bool LedSet;

    TimeOut -= Parameter.IntervalSec;

    SpNavData NavData;
    String NmeaString = "";

    /* Blink LED. */
    Led_isActive();

    int WriteRequest = false;

    /* Check update. */
    if (Gnss.waitUpdate(Parameter.IntervalSec * 1000))
    {
      /* Get NavData. */
      Gnss.getNavData(&NavData);

      LedSet = ((NavData.posDataExist) && (NavData.posFixMode != 1));	// 初回POSFIXが分かる？
      if(PosFixflag != LedSet)
      {
        Led_isPosfix(LedSet);
        PosFixflag = LedSet;

        if(LedSet == true)
        {
          TimeOut = SetActiveSec;
          WriteRequest = true;
        }
      }

      /* Get Nmea Data. */
      NmeaString = getNmeaGga(&NavData);
      if (strlen(NmeaString.c_str()) == 0)
      {
        /* Error case. */
        APP_PRINT_E("getNmea error");
        Led_isError(true);
      }
      else
      {
        /* Output Nmea Data. */
        if (Parameter.NmeaOutUart == true)
        {
          /* To Uart. */
          APP_PRINT(NmeaString.c_str());
        }

        if (Parameter.NmeaOutFile == true)
        {
          /* To SDCard. */
          BuffSize = NMEA_BUFFER_SIZE * (IDLE_ACTIVE_TIME / Parameter.IntervalSec);

          if (pNmeaBuff == NULL)
          {
            /* Alloc buffer. */
            pNmeaBuff = (char*)malloc(BuffSize);
            if (pNmeaBuff != NULL)
            {
              /* Clear Buffer */
              pNmeaBuff[0] = 0x00;
            }
          }

          if (pNmeaBuff != NULL)
          {
            /* Store Nmea Data to buffer. */
            strncat(pNmeaBuff, NmeaString.c_str(), BuffSize);
          }
        }

        /* Output Binary Data. */
        if (Parameter.BinaryOut == true)
        {
          BuffSize = Gnss.getPositionDataSize();
          if (pBinaryBuffer == NULL)
          {
            /* Alloc buffer. */
            pBinaryBuffer = (char*)malloc(Gnss.getPositionDataSize());
            if (pBinaryBuffer != NULL)
            {
              /* Clear Buffer. */
              pBinaryBuffer[0] = 0x00;
            }
          }
          if (pBinaryBuffer != NULL)
          {

            if (Gnss.getPositionData(pBinaryBuffer) == BuffSize)
            {
              /* Write Binary Data. */
              GnssPositionData *pAdr = (GnssPositionData*)pBinaryBuffer;
              Led_isSdAccess(true);
              WriteSize = WriteBinary((char*)&pAdr->MagicNumber, FilenameBin, sizeof(pAdr->MagicNumber), (FILE_WRITE | O_APPEND));
              WriteSize = WriteBinary((char*)&pAdr->Data,        FilenameBin, sizeof(pAdr->Data),        (FILE_WRITE | O_APPEND));
              WriteSize = WriteBinary((char*)&pAdr->CRC,         FilenameBin, sizeof(pAdr->CRC),         (FILE_WRITE | O_APPEND));
              Led_isSdAccess(false);

              /* Check result. */
              if (WriteSize != BuffSize)
              {
                Led_isError(true);
              }
            }
          }
        }
      }
    }

    /* Counter Check. */
    if (TimeOut <= 0)
    {
      if (Parameter.SleepSec > 0)
      {
        /* Set new mode. */
        State = eStateSleep;

        /* Go to Sleep mode. */
        SleepIn();

        /* Set sleep timeout. */
        TimeOut = Parameter.SleepSec;
      }
      else
      {
        /* Set sleep timeout. */
        TimeOut = Parameter.SleepSec;
      }

      WriteRequest = true;
    }

    /* Check NMEA buffer. */
    if(strlen(pNmeaBuff) > (BuffSize - NMEA_BUFFER_SIZE))
    {
      /* There is no capacity for writing in the next NMEA. */
    }

    /* Write NMEA data. */
    if(WriteRequest == true)
    {
      if (pNmeaBuff != NULL)
      {
        /* Write Nmea Data. */
        Led_isSdAccess(true);
        WriteSize = WriteChar(pNmeaBuff, FilenameTxt, (FILE_WRITE | O_APPEND));
        Led_isSdAccess(false);

        /* Check result. */
        if (WriteSize != strlen(pNmeaBuff))
        {
          Led_isError(true);
        }

        /* Clear Buffer */
        pNmeaBuff[0] = 0x00;
      }
    }
  }
}
