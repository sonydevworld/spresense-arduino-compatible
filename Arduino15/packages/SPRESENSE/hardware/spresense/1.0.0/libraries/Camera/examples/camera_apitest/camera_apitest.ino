/*
 *  camera_apitest.ino - Advanced camera API example sketch
 *  Copyright 2022 Sony Semiconductor Solutions Corporation
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
 *  This is a test app for the camera library.
 *  This library can only be used on the Spresense with the FCBGA chip package.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <Camera.h>
#include <SDHCI.h>
#include <RTC.h>


/****************************************************************************
 * LCD preview option
 ****************************************************************************/

/* Use Adafruit_ILI9341 library and Adafruit-GFX-Library
 * Please see Spresense Arduino Tutorial for more details.
 * https://developer.sony.com/develop/spresense/docs/arduino_tutorials_ja.html
 */

//#define USE_LIB_ADAFRUIT_ILI9341

#ifdef USE_LIB_ADAFRUIT_ILI9341
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS -1
#define TFT_RST 8
#define TFT_DC  9
#define TFT_ROTATION 3

Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI, TFT_DC, TFT_CS, TFT_RST);
#endif

/****************************************************************************
 * Initial parameters
 ****************************************************************************/
SDClass  theSD;
int               g_pict_id = 0;
int               g_width   = CAM_IMGSIZE_QUADVGA_H;
int               g_height  = CAM_IMGSIZE_QUADVGA_V;
CAM_IMAGE_PIX_FMT g_img_fmt = CAM_IMAGE_PIX_FMT_JPG;
CAM_WHITE_BALANCE g_wb      = CAM_WHITE_BALANCE_AUTO;
CAM_COLOR_FX      g_cfx     = CAM_COLOR_FX_NONE;
bool              g_awb     = true;
bool              g_ae      = true;
int               g_divisor = 7;

/****************************************************************************
 * Print error message
 ****************************************************************************/
void printError(enum CamErr err)
{
  Serial.print("Error: ");
  switch (err) {
  case CAM_ERR_NO_DEVICE:             Serial.println("No Device");                     break;
  case CAM_ERR_ILLEGAL_DEVERR:        Serial.println("Illegal device error");          break;
  case CAM_ERR_ALREADY_INITIALIZED:   Serial.println("Already initialized");           break;
  case CAM_ERR_NOT_INITIALIZED:       Serial.println("Not initialized");               break;
  case CAM_ERR_NOT_STILL_INITIALIZED: Serial.println("Still picture not initialized"); break;
  case CAM_ERR_CANT_CREATE_THREAD:    Serial.println("Failed to create thread");       break;
  case CAM_ERR_INVALID_PARAM:         Serial.println("Invalid parameter");             break;
  case CAM_ERR_NO_MEMORY:             Serial.println("No memory");                     break;
  case CAM_ERR_USR_INUSED:            Serial.println("Buffer already in use");         break;
  case CAM_ERR_NOT_PERMITTED:         Serial.println("Operation not permitted");       break;
  default:
    break;
  }
}

/****************************************************************************
 * Callback from Camera library when video frame is captured.
 ****************************************************************************/
void CamCB(CamImage img)
{
  /* Check the img instance is available or not. */
  if (img.isAvailable()) {
      /* If you want RGB565 data, convert image data format to RGB565 */
      img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);

#ifdef USE_LIB_ADAFRUIT_ILI9341
      /* You can use image data directly by using getImgSize() and getImgBuff().
       * for displaying image to a display, etc. */
      tft.drawRGBBitmap(0, 0, (uint16_t *)img.getImgBuff(), 320, 240);
#endif
  } else {
    Serial.println("Failed to get video stream image");
  }
}

/****************************************************************************
 * setup
 ****************************************************************************/
void setup()
{
  CamErr err;

  Serial.begin(115200);

  /* Set compiled datetime to RTC temporarily */
  RTC.begin();
  RtcTime compiledDateTime(__DATE__, __TIME__);
  RTC.setTime(compiledDateTime);

#ifdef USE_LIB_ADAFRUIT_ILI9341
  /* Initialize TFT */
  tft.begin();
  tft.setRotation(TFT_ROTATION);
#endif

  /* Initialize SD */
  while (!theSD.begin()) {
    /* wait until SD card is mounted. */
    Serial.println("Insert SD card.");
    sleep(1);
  }

  /* begin() without parameters means that
   * number of buffers = 1, 30FPS, QVGA, YUV 4:2:2 format */

  Serial.println("Prepare camera");
  err = theCamera.begin();
  if (err != CAM_ERR_SUCCESS) {
    printError(err);
  }

  /* Start video stream. */

  Serial.println("Start streaming");
  err = theCamera.startStreaming(true, CamCB);
  if (err != CAM_ERR_SUCCESS) {
    printError(err);
  }

  /* Set parameters about still picture. */

  Serial.println("Set still picture format");
  err = theCamera.setStillPictureImageFormat(g_width, g_height,
                                             g_img_fmt, g_divisor);
  if (err != CAM_ERR_SUCCESS) {
    printError(err);
  }
}

/****************************************************************************
 * Get the input string from serial monitor
 ****************************************************************************/
String inputString()
{
  String input;

  do {
    if (Serial.available() > 0 ) {
      input = Serial.readStringUntil('\n');
      input.toUpperCase();
      input.trim();
    }
  } while (0 == input.length());

  return input;
}

/****************************************************************************
 * API: getHDR()
 ****************************************************************************/
void showHDR()
{
  CAM_HDR_MODE hdr = theCamera.getHDR();

  Serial.print("HDR          : ");
  switch (hdr) {
  case CAM_HDR_MODE_OFF:  Serial.println("Off");   break;
  case CAM_HDR_MODE_AUTO: Serial.println("Auto");  break;
  case CAM_HDR_MODE_ON:   Serial.println("On");    break;
  default:                Serial.println("Error"); break;
  }
}

/****************************************************************************
 * API: setHDR()
 ****************************************************************************/
void menuHDR()
{
  CamErr err;
  String input;
  CAM_HDR_MODE mode;

  while (1) {
    showHDR();
    Serial.println();
    Serial.println("--- HDR Menu ---");
    Serial.println("0: Off");
    Serial.println("1: Auto");
    Serial.println("2: On");
    Serial.println("X: exit");
    Serial.println();

    input = inputString();

    if      (input == "0") { mode = CAM_HDR_MODE_OFF;  }
    else if (input == "1") { mode = CAM_HDR_MODE_AUTO; }
    else if (input == "2") { mode = CAM_HDR_MODE_ON;   }
    else                   { return; }

    err = theCamera.setHDR(mode);

    if (err != CAM_ERR_SUCCESS) {
      printError(err);
    }
  }
}

/****************************************************************************
 * Show AWB current settings
 ****************************************************************************/
void showAWB()
{
  Serial.printf("AWB          : %s\n", g_awb ? "On" : "Off");

  Serial.print("AWB mode     : ");
  switch (g_wb) {
  case CAM_WHITE_BALANCE_AUTO:         Serial.println("Automatic");    break;
  case CAM_WHITE_BALANCE_INCANDESCENT: Serial.println("Incandescent"); break;
  case CAM_WHITE_BALANCE_FLUORESCENT:  Serial.println("Fluorescent");  break;
  case CAM_WHITE_BALANCE_DAYLIGHT:     Serial.println("Daylight");     break;
  case CAM_WHITE_BALANCE_FLASH:        Serial.println("Flash");        break;
  case CAM_WHITE_BALANCE_CLOUDY:       Serial.println("Cloudy");       break;
  case CAM_WHITE_BALANCE_SHADE:        Serial.println("Shade");        break;
  default:
    break;
  }
}

/****************************************************************************
 * API: setAutoWhiteBalance(), setAutoWhiteBalanceMode()
 ****************************************************************************/
void menuAWB()
{
  CamErr err;
  String input;
  bool awb = g_awb;
  CAM_WHITE_BALANCE wb = g_wb;

  while (1) {
    showAWB();
    Serial.println();
    Serial.println("--- Auto WhiteBlance Menu ---");
    Serial.println("Y: On");
    Serial.println("N: Off");
    Serial.println("0: Automatic");
    Serial.println("1: Incandescent");
    Serial.println("2: Fluorescent");
    Serial.println("3: Daylight");
    Serial.println("4: Flash");
    Serial.println("5: Cloudy");
    Serial.println("6: Shade");
    Serial.println("X: exit");
    Serial.println();

    input = inputString();

    if ((input == "Y") || (input == "N")) {
      if      (input == "Y") { Serial.println("On");  awb = true;  }
      else if (input == "N") { Serial.println("Off"); awb = false; }

      err = theCamera.setAutoWhiteBalance(awb);

    } else {
      if      (input == "0") { Serial.println("Automatic");    wb = CAM_WHITE_BALANCE_AUTO;         }
      else if (input == "1") { Serial.println("Incandescent"); wb = CAM_WHITE_BALANCE_INCANDESCENT; }
      else if (input == "2") { Serial.println("Fluorescent");  wb = CAM_WHITE_BALANCE_FLUORESCENT;  }
      else if (input == "3") { Serial.println("Daylight");     wb = CAM_WHITE_BALANCE_DAYLIGHT;     }
      else if (input == "4") { Serial.println("Flash");        wb = CAM_WHITE_BALANCE_FLASH;        }
      else if (input == "5") { Serial.println("Cloudy");       wb = CAM_WHITE_BALANCE_CLOUDY;       }
      else if (input == "6") { Serial.println("Shade");        wb = CAM_WHITE_BALANCE_SHADE;        }
      else { return; }

      err = theCamera.setAutoWhiteBalanceMode(wb);
    }

    if (err != CAM_ERR_SUCCESS) {
      printError(err);
    } else {
      g_awb = awb;
      g_wb = wb;
    }
  }
}

/****************************************************************************
 * Show Color Effect current setting
 ****************************************************************************/
void showCFX()
{
  Serial.print("Color Effect : ");
  switch (g_cfx) {
  case CAM_COLOR_FX_NONE:         Serial.println("No effect");         break;
  case CAM_COLOR_FX_BW:           Serial.println("Black/white");       break;
  case CAM_COLOR_FX_SEPIA:        Serial.println("Sepia");             break;
  case CAM_COLOR_FX_NEGATIVE:     Serial.println("Pos/Neg inversion"); break;
  case CAM_COLOR_FX_SKETCH:       Serial.println("Sketch");            break;
  case CAM_COLOR_FX_VIVID:        Serial.println("Vivid");             break;
  case CAM_COLOR_FX_SOLARIZATION: Serial.println("Solarization");      break;
  case CAM_COLOR_FX_PASTEL:       Serial.println("Pastel");            break;
  default:
    break;
  }
}

/****************************************************************************
 * API: setColorEffect()
 ****************************************************************************/
void menuCFX()
{
  CamErr err;
  String input;
  CAM_COLOR_FX cfx;

  while (1) {
    showCFX();
    Serial.println();
    Serial.println("--- Color Effect Menu ---");
    Serial.println("0 : No effect");
    Serial.println("1 : Black/white");
    Serial.println("2 : Sepia");
    Serial.println("3 : Positive/Negative inversion");
    Serial.println("4 : Sketch");
    Serial.println("5 : Vivid");
    Serial.println("6 : Solarization");
    Serial.println("7 : Pastel");
    Serial.println("X : exit");
    Serial.println();

    input = inputString();

    if      (input == "0") { Serial.println("No effect");         cfx = CAM_COLOR_FX_NONE;         }
    else if (input == "1") { Serial.println("Black/white");       cfx = CAM_COLOR_FX_BW;           }
    else if (input == "2") { Serial.println("Sepia");             cfx = CAM_COLOR_FX_SEPIA;        }
    else if (input == "3") { Serial.println("Pos/Neg inversion"); cfx = CAM_COLOR_FX_NEGATIVE;     }
    else if (input == "4") { Serial.println("Sketch");            cfx = CAM_COLOR_FX_SKETCH;       }
    else if (input == "5") { Serial.println("Vivid");             cfx = CAM_COLOR_FX_VIVID;        }
    else if (input == "6") { Serial.println("Solarization");      cfx = CAM_COLOR_FX_SOLARIZATION; }
    else if (input == "7") { Serial.println("Pastel");            cfx = CAM_COLOR_FX_PASTEL;       }
    else { return; }

    err = theCamera.setColorEffect(cfx);

    if (err != CAM_ERR_SUCCESS) {
      printError(err);
    } else {
      g_cfx = cfx;
    }
  }
}

/****************************************************************************
 * API: getAbsoluteExposure()
 ****************************************************************************/
void showAE()
{
  int32_t time = theCamera.getAbsoluteExposure();

  Serial.printf("AE           : %s\n", g_ae ? "Auto" : "Manual");
  Serial.printf("AE time      : %ld [100us]\n", time);
}

/****************************************************************************
 * API: setAutoExposure(), setAbsoluteExposure()
 ****************************************************************************/
void menuAE()
{
  CamErr err;
  String input;
  bool ae = g_ae;

  while (1) {
    showAE();
    Serial.println();
    Serial.println("--- AE Menu ---");
    Serial.println("A : Auto");
    Serial.println("M : Manual");
    Serial.println("1~: Exposure time [100usec]");
    Serial.println("G : get");
    Serial.println("X : exit");
    Serial.println();

    input = inputString();

    if ((input == "A") || (input == "M")) {
      if      (input == "A") { Serial.println("Auto");   ae = true;  }
      else if (input == "M") { Serial.println("Manual"); ae = false; }

      err = theCamera.setAutoExposure(ae);

    } else if (input == "G") {
      continue;
    } else if (isDigit(input.charAt(0))) {
      long value = input.toInt();

      err = theCamera.setAbsoluteExposure(value);
      ae = false;
    } else {
      return;
    }

    if (err != CAM_ERR_SUCCESS) {
      printError(err);
    } else {
      g_ae = ae;
    }

    /* If AE is changed, it takes time for the value to take effect. */
    delay(250);

  }
}

/****************************************************************************
 * API: getISOSensitivity()
 ****************************************************************************/
void showISO()
{
  int32_t iso = theCamera.getISOSensitivity();

  Serial.printf("ISO          : %ld\n", iso / 1000);
}

/****************************************************************************
 * API: setISOSensitivity()
 ****************************************************************************/
void menuISO()
{
  CamErr err;
  String input;

  while (1) {
    showISO();
    Serial.println();
    Serial.println("--- ISO Menu ---");
    Serial.println("A   : Auto");
    Serial.println("N   : Off");
    Serial.println("25  : ISO Sensitivity 25");
    Serial.println("32  : ISO Sensitivity 32");
    Serial.println("40  : ISO Sensitivity 40");
    Serial.println("50  : ISO Sensitivity 50");
    Serial.println("64  : ISO Sensitivity 64");
    Serial.println("80  : ISO Sensitivity 80");
    Serial.println("100 : ISO Sensitivity 100");
    Serial.println("125 : ISO Sensitivity 125");
    Serial.println("160 : ISO Sensitivity 160");
    Serial.println("200 : ISO Sensitivity 200");
    Serial.println("250 : ISO Sensitivity 250");
    Serial.println("320 : ISO Sensitivity 320");
    Serial.println("400 : ISO Sensitivity 400");
    Serial.println("500 : ISO Sensitivity 500");
    Serial.println("640 : ISO Sensitivity 640");
    Serial.println("800 : ISO Sensitivity 800");
    Serial.println("1000: ISO Sensitivity 1000");
    Serial.println("1250: ISO Sensitivity 1250");
    Serial.println("1600: ISO Sensitivity 1600");
    Serial.println("G   : get");
    Serial.println("X   : exit");
    Serial.println();

    input = inputString();

    if      (input == "A") { err = theCamera.setAutoISOSensitivity(true);  }
    else if (input == "N") { err = theCamera.setAutoISOSensitivity(false); }
    else if (input == "G") { continue; }
    else if (isDigit(input.charAt(0))) {
      int value = (int)input.toInt();
      err = theCamera.setISOSensitivity(value * 1000);
    } else {
      return;
    }

    if (err != CAM_ERR_SUCCESS) {
      printError(err);
    }

    /* If AE is changed, it takes time for the value to take effect. */
    delay(250);

  }
}

/****************************************************************************
 * API: setStillPictureImageFormat(img_fmt)
 ****************************************************************************/
void menuFormat()
{
  CamErr err;
  String input;
  CAM_IMAGE_PIX_FMT fmt;

  while (1) {
    showFormat();
    Serial.println();
    Serial.println("--- Format Menu ---");
    Serial.println("RGB : RGB565");
    Serial.println("YUV : YUV422");
    Serial.println("JPG : JPEG");
    Serial.println("X   : exit");
    Serial.println();

    input = inputString();

    if      (input == "RGB") { fmt = CAM_IMAGE_PIX_FMT_RGB565; }
    else if (input == "YUV") { fmt = CAM_IMAGE_PIX_FMT_YUV422; }
    else if (input == "JPG") { fmt = CAM_IMAGE_PIX_FMT_JPG;    }
    else {
      return;
    }

    err = theCamera.setStillPictureImageFormat(g_width, g_height,
                                               fmt, g_divisor);
    if (err != CAM_ERR_SUCCESS) {
      printError(err);
    } else {
      g_img_fmt = fmt;
    }
  }
}

/****************************************************************************
 * API: setStillPictureImageFormat(img_width, img_height)
 ****************************************************************************/
void menuResolution()
{
  CamErr err;
  String input;
  int width;
  int height;

  while (1) {
    showFormat();
    Serial.println();
    Serial.println("--- Image Size Menu ---");
    Serial.println("1  :        (96x64)");
    Serial.println("2  : QQVGA  (160x120)");
    Serial.println("3  : QVGA   (320x240)");
    Serial.println("4  : VGA    (640x480)");
    Serial.println("5  : HD     (1280x720)");
    Serial.println("6  : QUADVGA(1280x960)");
    Serial.println("7  : FULLHD (1920x1080)");
    Serial.println("8  : QXGA   (2048x1536)");
    Serial.println("9  : 5MP    (2560x1920)");
    Serial.println("X  : exit");
    Serial.println();

    input = inputString();

    if      (input == "1") { width = 96;                    height = 64;                    }
    else if (input == "2") { width = CAM_IMGSIZE_QQVGA_H;   height = CAM_IMGSIZE_QQVGA_V;   }
    else if (input == "3") { width = CAM_IMGSIZE_QVGA_H;    height = CAM_IMGSIZE_QVGA_V;    }
    else if (input == "4") { width = CAM_IMGSIZE_VGA_H;     height = CAM_IMGSIZE_VGA_V;     }
    else if (input == "5") { width = CAM_IMGSIZE_HD_H;      height = CAM_IMGSIZE_HD_V;      }
    else if (input == "6") { width = CAM_IMGSIZE_QUADVGA_H; height = CAM_IMGSIZE_QUADVGA_V; }
    else if (input == "7") { width = CAM_IMGSIZE_FULLHD_H;  height = CAM_IMGSIZE_FULLHD_V;  }
    else if (input == "8") { width = CAM_IMGSIZE_3M_H;      height = CAM_IMGSIZE_3M_V;      }
    else if (input == "9") { width = CAM_IMGSIZE_5M_H;      height = CAM_IMGSIZE_5M_V;      }
    else {
      return;
    }

    /* Adjust the divisor until buffer is allocated. */
    while (1) {
      err = theCamera.setStillPictureImageFormat(width, height,
                                                 g_img_fmt, g_divisor);
      if (err == CAM_ERR_NO_MEMORY) {
        g_divisor++;
      } else {
        break;
      }
    }

    if (err != CAM_ERR_SUCCESS) {
      printError(err);
    } else {
      g_width  = width;
      g_height = height;
    }
  }
}

/****************************************************************************
 * API: setStillPictureImageFormat(jpgbufsize_divisor)
 ****************************************************************************/
void menuMemorySize()
{
  CamErr err;
  String input;
  int bufSize;
  int divisor;
  int i;

  if (g_img_fmt != CAM_IMAGE_PIX_FMT_JPG) {
    return;
  }

  while (1) {

    bufSize = g_width * g_height * 2;

    Serial.println();
    Serial.printf("Divisor: %d\n", g_divisor);
    Serial.printf("BufSize: %.2f [KB]\n", bufSize / g_divisor / 1024.0);
    Serial.println();
    Serial.println("--- Allocate Memory Size Menu ---");
    Serial.println("1~");
    for (i = g_divisor - 2; i < g_divisor + 4; i++) {
      if (i > 0) {
        Serial.printf("%d : BufSize: %.2f [KB]\n", i, bufSize / i / 1024.0);
      }
    }
    Serial.println("X : exit");
    Serial.println();

    input = inputString();

    if (isDigit(input.charAt(0))) {
      divisor = (int)input.toInt();
    } else {
      return;
    }

    /* Adjust the divisor until buffer is allocated. */
    while (1) {
      err = theCamera.setStillPictureImageFormat(g_width, g_height,
                                                 g_img_fmt, divisor);
      if (err == CAM_ERR_NO_MEMORY) {
        divisor++;
      } else {
        break;
      }
    }

    if (err != CAM_ERR_SUCCESS) {
      printError(err);
    } else {
      g_divisor = divisor;
    }
  }
}

/****************************************************************************
 * API: getJPEGQuality()
 ****************************************************************************/
void showJPEGQuality()
{
  int q = theCamera.getJPEGQuality();

  Serial.printf("JPEG Quality : %d [%%]\n", q);
}

/****************************************************************************
 * API: setJPEGQuality()
 ****************************************************************************/
void menuJPEGQuality()
{
  CamErr err;
  String input;

  while (1) {
    showJPEGQuality();
    Serial.println();
    Serial.println("--- JPEG Quality Menu ---");
    Serial.println("1-100: JPEG quality");
    Serial.println("X : exit");
    Serial.println();

    input = inputString();

    if (isDigit(input.charAt(0))) {
      int value = (int)input.toInt();
      err = theCamera.setJPEGQuality(value);
    } else {
      return;
    }

    if (err != CAM_ERR_SUCCESS) {
      printError(err);
    }
  }
}

/****************************************************************************
 * Set the picture index number
 ****************************************************************************/
void menuPictIndex()
{
  String input;

  while (1) {
    Serial.println(String("Picture Index: ") + g_pict_id);
    Serial.println();
    Serial.println("--- Set Picture Index Menu ---");
    Serial.println("0-: Picture Index");
    Serial.println("X : exit");
    Serial.println();

    input = inputString();

    if (isDigit(input.charAt(0))) {
      g_pict_id = (int)input.toInt();
    } else {
      return;
    }
  }
}

/****************************************************************************
 * Show the picture format
 ****************************************************************************/
void showFormat()
{
  Serial.printf("Image Format : %s (%dx%d)\n",
                (g_img_fmt == CAM_IMAGE_PIX_FMT_RGB565) ? "RGB565" :
                (g_img_fmt == CAM_IMAGE_PIX_FMT_YUV422) ? "YUV422" :
                (g_img_fmt == CAM_IMAGE_PIX_FMT_JPG)    ? "JPEG" : "NONE",
                g_width, g_height);
}

/****************************************************************************
 * API: getFrameInterval()
 ****************************************************************************/
void showFrameInterval()
{
  int time = theCamera.getFrameInterval();

  Serial.printf("FrameInterval: %d [100us]\n", time);
}

/****************************************************************************
 * API: getDeviceType()
 ****************************************************************************/
void showDevice()
{
  CAM_DEVICE_TYPE type = theCamera.getDeviceType();

  Serial.print("Device Type  : ");
  switch (type) {
  case CAM_DEVICE_TYPE_UNKNOWN: Serial.println("Unknown"); break;
  case CAM_DEVICE_TYPE_ISX012:  Serial.println("ISX012");  break;
  case CAM_DEVICE_TYPE_ISX019:  Serial.println("ISX019");  break;
  default:                                                 break;
  }
}

/****************************************************************************
 * Show the current settings
 ****************************************************************************/
void menuInfo()
{
  Serial.println();
  Serial.println("--- Settings Info ---");
  showDevice();
  showFormat();
  showJPEGQuality();
  showHDR();
  showAE();
  showAWB();
  showISO();
  showCFX();
  showFrameInterval();
}

/****************************************************************************
 * API: takePicture()
 ****************************************************************************/
void takePicture()
{
  char filename[32] = {0};

  Serial.println();
  Serial.println("--- Take Picture ---");

  CamImage img = theCamera.takePicture();

  if (img.isAvailable()) {
    sprintf(filename, "PICT%04d.%s", g_pict_id++,
            (img.getPixFormat() == CAM_IMAGE_PIX_FMT_RGB565) ? "RGB565" :
            (img.getPixFormat() == CAM_IMAGE_PIX_FMT_YUV422) ? "YUV422" :
            (img.getPixFormat() == CAM_IMAGE_PIX_FMT_JPG)    ? "JPG" :
            (img.getPixFormat() == CAM_IMAGE_PIX_FMT_GRAY)   ? "GRAY" : "NONE");

    /* If the same filename exists, remove it in advance to prevent file appending. */
    theSD.remove(filename);

    File myFile = theSD.open(filename, FILE_WRITE);
    myFile.write(img.getImgBuff(), img.getImgSize());
    myFile.close();

    Serial.printf("Save: %s\n", filename);
    Serial.printf("Resolution: %dx%d\n", img.getWidth(), img.getHeight());
    Serial.printf("MemorySize: %.2f / %.2f [KB]\n",
                  img.getImgSize() / 1024.0, img.getImgBuffSize() / 1024.0);
  } else {
    Serial.println("Error: Failed to take picture.");
    Serial.println("Increase the size of memory allocated.");
    Serial.println("or Decrease the JPEG Quality.");
  }
}

/****************************************************************************
 * Set the current time
 ****************************************************************************/
int getValueInt(int s, int e)
{
  String input;
  int value;

  Serial.printf("Input from \"%d\" to \"%d\"\n", s, e);

  input = inputString();
  if (isDigit(input.charAt(0))) {
    value = (int)input.toInt();
    if ((s <= value) && (value <= e)) {
      Serial.println(value);
      return value;
    }
  }
  return -1;
}

void menuRTC()
{
  RtcTime rtc;
  String input;
  int value;

  while (1) {
    value = -1;

    rtc = RTC.getTime();
    Serial.printf("%04d/%02d/%02d %02d:%02d:%02d\n",
                  rtc.year(), rtc.month(), rtc.day(),
                  rtc.hour(), rtc.minute(), rtc.second());
    Serial.println();
    Serial.println("--- RTC setting Menu ---");
    Serial.println("YY: Year");
    Serial.println("MM: Month");
    Serial.println("DD: Day");
    Serial.println("H : Hour");
    Serial.println("M : Minute");
    Serial.println("S : Second");
    Serial.println("G : Get Current Time");
    Serial.println("X : exit");
    Serial.println();

    input = inputString();

    if (input == "YY")  {
      value = getValueInt(2000, 2037);
      if (value > 0) rtc.year(value);
    } else if (input == "MM") {
      value = getValueInt(1, 12);
      if (value > 0) rtc.month(value);
    } else if (input == "DD") {
      value = getValueInt(1, 31);
      if (value > 0) rtc.day(value);
    } else if (input == "H") {
      value = getValueInt(0, 23);
      if (value > 0) rtc.hour(value);
    } else if (input == "M") {
      value = getValueInt(0, 59);
      if (value >= 0) rtc.minute(value);
    } else if (input == "S") {
      value = getValueInt(0, 59);
      if (value >= 0) rtc.second(value);
    } else if (input == "G") {
      continue;
    } else {
      return;
    }

    if (value >= 0) {
      RTC.setTime(rtc);
    }
  }
}

/****************************************************************************
 * Top Menu
 ****************************************************************************/
void menuTop()
{
  Serial.println();
  Serial.println("--- Top Menu ---");
  Serial.println("I    : Show Information");
  Serial.println("HDR  : Enter HDR Menu");
  Serial.println("AE   : Enter Auto Exposure Menu");
  Serial.println("AWB  : Enter Auto WhiteBalance Menu");
  Serial.println("ISO  : Enter ISO sensitivity Menu");
  Serial.println("CFX  : Enter Color Effect Menu");
  Serial.println("QTY  : Enter JPEG Quality Menu");
  Serial.println("FMT  : Enter Picture Format Menu");
  Serial.println("RES  : Enter Picture Resolution Menu");
  Serial.println("IDX  : Enter Picture Index Menu");
  Serial.println("MEM  : Enter Allocated Memory Size Menu");
  Serial.println("RTC  : Enter RTC setting Menu");
  Serial.println("P    : Take Picture");
  Serial.println();

  String input = inputString();

  if      (input == "I")    { menuInfo();        }
  else if (input == "HDR")  { menuHDR();         }
  else if (input == "AE")   { menuAE();          }
  else if (input == "AWB")  { menuAWB();         }
  else if (input == "ISO")  { menuISO();         }
  else if (input == "CFX")  { menuCFX();         }
  else if (input == "QTY")  { menuJPEGQuality(); }
  else if (input == "FMT")  { menuFormat();      }
  else if (input == "RES")  { menuResolution();  }
  else if (input == "IDX")  { menuPictIndex();   }
  else if (input == "MEM")  { menuMemorySize();  }
  else if (input == "RTC")  { menuRTC();         }
  else if (input == "P")    { takePicture();     }
}

/****************************************************************************
 * loop
 ****************************************************************************/
void loop()
{
  menuTop();
}
