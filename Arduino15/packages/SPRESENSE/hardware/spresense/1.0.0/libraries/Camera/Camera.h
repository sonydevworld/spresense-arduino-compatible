/*
 *  Camera.h - Camera include file for the Spresense SDK
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
 * Camera Library for Arduino IDE on Spresense.
 *
 * Spresense上のArduino 向け Camera Library Classです。
 * こちらのライブラリを使用することで、Spresense上での
 *  - 写真撮影
 * が可能になります。
 *
 */

#ifndef Camera_h
#define Camera_h

#include <SDHCI.h>

#ifdef __cplusplus

#include <nuttx/arch.h>
#include <nuttx/board.h>
#include <video/isx012.h>
#include <video/video.h>
#include <asmp/mpshm.h>

#define err_t int


/*--------------------------------------------------------------------------*/
/**
 * Camera Library Error Code Definitions.
 */


/*--------------------------------------------------------------------------*/
/**
 * Camera Library Type Definitions.
 */


/*--------------------------------------------------------------------------*/

/**
 * Camera Library Class Definitions.
 */

class CameraClass
{
public:

  /**
   * Initialize the camera library and HW modules.
   *
   * この関数は、Cameraライブラリを使用する際に、1回だけ呼び出します。
   * 複数回呼び出すと、エラーが発生しますが、"end()" を呼び出した場合、再度、この関数を呼ぶ必要があります。
   *
   * 本関数の中で、を行っていきます。
   *
   */
  err_t begin(void);

  /**
   * Finalization the camera library and HW modules.
   *
   * この関数は、Cameraライブラリがbeginを呼ばれて、活性化された際に1回だけ呼び出します。
   * "begin()"を呼ぶ前に呼んでしまったり、複数回呼び出すと、エラーが発生します。
   *
   * 本関数の中で、を行っていきます。
   *
   */
  err_t end(void);

  /**
   * Initiraize Camera Library.
   *
   * この関数は、Cameraライブラリの初期化を行うものです。
   *
   */
  err_t initialize();

  /**
   * Read Camera Library.
   *
   *
   * 本関数の中でcaptureを行い、captureデータのアドレスおよびcaptureサイズを
   * OUTパラメータで返します。
   *
   */
  err_t read(uint8_t** addr, int *size);

  /**
   * Read Camera Library and save to sdcard.
   *
   *
   * 本関数の中でcaptureおよび、SDカードへの保存を行っています。
   *
   */
  err_t read(File&);

//private:

  /**
   * To avoid create multiple instance
   */

/*  CammeraClass() {}
  CammeraClass(const CameraClass&);
  CameraClass& operator=(const CameraClass&);
  ~CameraClass() {}
*/

private:
  mpshm_t        shm;
  uint8_t        *buffer;
  int fd;
};

extern CameraClass Camera;

#endif //__cplusplus
#endif //Camera_h

