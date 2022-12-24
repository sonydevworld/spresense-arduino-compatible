/*
 *  Camera.h - Camera include file for the Spresense SDK
 *  Copyright 2018, 2020-2022 Sony Semiconductor Solutions Corporation
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
 * @file Camera.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Camera Library for Arduino IDE on Spresense.
 * @details By using this library, you can use the follow features on SPRESENSE.
 *           - Get Picture from Camera module as a Jpeg format data.
 *           - Get Image from Camera module for preview image.
 *          このライブラリを使うことで、以下の機能をSPRESENSE上で利用することが出来ます。
 *           - SPRESENSE CameraモジュールからJpegフォーマットデータとして写真を取得
 *           - SPRESENSE Cameraモジュールからプレビューイメージとして画像を取得
 */

#ifndef __SPRESENSE_CAMERA_CLASS_H__
#define __SPRESENSE_CAMERA_CLASS_H__

#ifdef SUBCORE
#error "Camera library is NOT supported by SubCore."
#endif

/**
 * @defgroup camera Camera Library API
 * @brief API for using Camera
 * @{
 */

#include <stdint.h>
#include <stdlib.h>

#include <semaphore.h>
#include <pthread.h>
#include <mqueue.h>

#include <nuttx/video/video.h>

class CameraClass;
class CamImage;

/**
 * @enum CAM_IMAGE_PIX_FMT
 * @brief [en] Camera Image Pixcel format <BR>
 *        [ja] Camera画像のピクセルフォーマット
 */
enum CAM_IMAGE_PIX_FMT {
  CAM_IMAGE_PIX_FMT_RGB565 = V4L2_PIX_FMT_RGB565, /**< RGB565 format */
  CAM_IMAGE_PIX_FMT_YUV422 = V4L2_PIX_FMT_UYVY,   /**< YUV422 packed. */
  CAM_IMAGE_PIX_FMT_JPG    = V4L2_PIX_FMT_JPEG,   /**< JPEG format */
  CAM_IMAGE_PIX_FMT_GRAY,                         /**< Gray-scale */
  CAM_IMAGE_PIX_FMT_NONE,                         /**< No defined format */
};


/**
 * @enum CamErr
 * @brief [en] Camera Error Codes. <BR>
 *        [ja] Cameraのエラーコード
 */
enum CamErr {
  CAM_ERR_SUCCESS               = 0,   /**< [en] Operation succeeded.                      <BR> [jp] 正常終了しました */
  CAM_ERR_NO_DEVICE             = -1,  /**< [en] No Video Device on this board.            <BR> [jp] Videoデバイスがありません */
  CAM_ERR_ILLEGAL_DEVERR        = -2,  /**< [en] Video Device detected error.              <BR> [jp] Videoデバイスがエラーを検出しました */
  CAM_ERR_ALREADY_INITIALIZED   = -3,  /**< [en] Library is already initialized            <BR> [jp] 既に初期化されています */
  CAM_ERR_NOT_INITIALIZED       = -4,  /**< [en] Library is not initialized                <BR> [jp] 初期化されていません */
  CAM_ERR_NOT_STILL_INITIALIZED = -5,  /**< [en] Still picture function is not initialized <BR> [jp] 静止画機能が初期化されていません */
  CAM_ERR_CANT_CREATE_THREAD    = -6,  /**< [en] Failed to create thread                   <BR> [jp] スレッド生成に失敗しました */
  CAM_ERR_INVALID_PARAM         = -7,  /**< [en] Invalid parameter is detected.            <BR> [jp] 不正なパラメータを検出しました */
  CAM_ERR_NO_MEMORY             = -8,  /**< [en] No memory on the device.                  <BR> [jp] メモリが足りません */
  CAM_ERR_USR_INUSED            = -9,  /**< [en] Buffer is using by user.                  <BR> [jp] バッファがユーザ使用中です */
  CAM_ERR_NOT_PERMITTED         = -10, /**< [en] Operation is not permitted.               <BR> [jp] 許容されていない操作です */
};

/**
 * @enum CAM_DEVICE_TYPE
 * @brief [en] Camera device type which is being used <BR>
 *        [ja] 使用されているカメラデバイスの種類
 */
enum CAM_DEVICE_TYPE {
  CAM_DEVICE_TYPE_UNKNOWN, /**< [en] Unknown <BR> [ja] 不明 */
  CAM_DEVICE_TYPE_ISX012,  /**< [en] ISX012  <BR> [ja] ISX012 */
  CAM_DEVICE_TYPE_ISX019,  /**< [en] ISX019  <BR> [ja] ISX019 */
};

/**
 * @enum CAM_WHITE_BALANCE
 * @brief [en] Camera White Balance setting parameters <BR>
 *        [ja] Cameraホワイトバランス設定値
 */
enum CAM_WHITE_BALANCE {
  CAM_WHITE_BALANCE_AUTO          = V4L2_WHITE_BALANCE_AUTO,          /**< [en] Automatic    <BR> [ja] 自動 */
  CAM_WHITE_BALANCE_INCANDESCENT  = V4L2_WHITE_BALANCE_INCANDESCENT,  /**< [en] Incandescent <BR> [ja] 白熱電球 */
  CAM_WHITE_BALANCE_FLUORESCENT   = V4L2_WHITE_BALANCE_FLUORESCENT,   /**< [en] Fluorescent  <BR> [ja] 蛍光灯 */
  CAM_WHITE_BALANCE_DAYLIGHT      = V4L2_WHITE_BALANCE_DAYLIGHT,      /**< [en] Daylight     <BR> [ja] 晴天 */
  CAM_WHITE_BALANCE_FLASH         = V4L2_WHITE_BALANCE_FLASH,         /**< [en] Flash        <BR> [ja] フラッシュ光 */
  CAM_WHITE_BALANCE_CLOUDY        = V4L2_WHITE_BALANCE_CLOUDY,        /**< [en] Cloudy       <BR> [ja] 曇り空 */
  CAM_WHITE_BALANCE_SHADE         = V4L2_WHITE_BALANCE_SHADE,         /**< [en] Shade        <BR> [ja] 影 */
};

/**
 * @defgroup CAM_IMGSIZE Camera Image size definitions
 * @brief Camera Image size definition.
 * @{
 */
#define CAM_IMGSIZE_QQVGA_H   (160)   /**< QQVGA    horizontal size */
#define CAM_IMGSIZE_QQVGA_V   (120)   /**< QQVGA    vertical   size */
#define CAM_IMGSIZE_QVGA_H    (320)   /**< QVGA     horizontal size */
#define CAM_IMGSIZE_QVGA_V    (240)   /**< QVGA     vertical   size */
#define CAM_IMGSIZE_VGA_H     (640)   /**< VGA      horizontal size */
#define CAM_IMGSIZE_VGA_V     (480)   /**< VGA      vertical   size */
#define CAM_IMGSIZE_HD_H      (1280)  /**< HD       horizontal size */
#define CAM_IMGSIZE_HD_V      (720)   /**< HD       vertical   size */
#define CAM_IMGSIZE_QUADVGA_H (1280)  /**< QUADVGA  horizontal size */
#define CAM_IMGSIZE_QUADVGA_V (960)   /**< QUADVGA  vertical   size */
#define CAM_IMGSIZE_FULLHD_H  (1920)  /**< FULLHD   horizontal size */
#define CAM_IMGSIZE_FULLHD_V  (1080)  /**< FULLHD   vertical   size */
#define CAM_IMGSIZE_3M_H      (2048)  /**< 3M       horizontal size */
#define CAM_IMGSIZE_3M_V      (1536)  /**< 3M       vertical   size */
#define CAM_IMGSIZE_5M_H      (2560)  /**< 5M       horizontal size */
#define CAM_IMGSIZE_5M_V      (1920)  /**< 5M       vertical   size */
/** @} */


/**
 * @enum CAM_SCENE_MODE
 * @brief [en] Camera Scene Mode setting parameters. <BR>
 *        [ja] Cameraのシーンモード設定値
 */
enum CAM_SCENE_MODE {
  CAM_SCENE_MODE_NONE         = V4L2_SCENE_MODE_NONE,         /**< [en] No Scene           <BR> [ja] シーン指定なし */
  CAM_SCENE_MODE_BACKLIGHT    = V4L2_SCENE_MODE_BACKLIGHT,    /**< [en] Under backlight    <BR> [ja] バックライトシーン */
  CAM_SCENE_MODE_BEACH_SNOW   = V4L2_SCENE_MODE_BEACH_SNOW,   /**< [en] Beach or Snow      <BR> [ja] ビーチ / 雪景色 */
  CAM_SCENE_MODE_CANDLE_LIGHT = V4L2_SCENE_MODE_CANDLE_LIGHT, /**< [en] Under candle light <BR> [ja] ロウソクのシーン */
  CAM_SCENE_MODE_DAWN_DUSK    = V4L2_SCENE_MODE_DAWN_DUSK,    /**< [en] Dawn dusk          <BR> [ja] 夕暮れ */
  CAM_SCENE_MODE_FALL_COLORS  = V4L2_SCENE_MODE_FALL_COLORS,  /**< [en] Fall colors        <BR> [ja] 紅葉 */
  CAM_SCENE_MODE_FIREWORKS    = V4L2_SCENE_MODE_FIREWORKS,    /**< [en] Fireworks          <BR> [ja] 花火シーン */
  CAM_SCENE_MODE_LANDSCAPE    = V4L2_SCENE_MODE_LANDSCAPE,    /**< [en] Landscape          <BR> [ja] ランドスケープ */
  CAM_SCENE_MODE_NIGHT        = V4L2_SCENE_MODE_NIGHT,        /**< [en] Night              <BR> [ja] 夜景 */
  CAM_SCENE_MODE_PARTY_INDOOR = V4L2_SCENE_MODE_PARTY_INDOOR, /**< [en] Party or Indoor    <BR> [ja] 室内 */
  CAM_SCENE_MODE_PORTRAIT     = V4L2_SCENE_MODE_PORTRAIT,     /**< [en] Portrait           <BR> [ja] ポートレート */
  CAM_SCENE_MODE_SPORTS       = V4L2_SCENE_MODE_SPORTS,       /**< [en] Sports             <BR> [ja] スポーツ */
  CAM_SCENE_MODE_SUNSET       = V4L2_SCENE_MODE_SUNSET,       /**< [en] Sunset             <BR> [ja] 日の入りシーン */
};


/**
 * @enum CAM_COLOR_FX
 * @brief [en] Camera Color effect setting parameters. <BR>
 *        [ja] Cameraの画像エフェクトの設定値
 */
enum CAM_COLOR_FX {
  CAM_COLOR_FX_NONE         = V4L2_COLORFX_NONE,           /**< [en] no effect                   <BR> [jp] 効果なし */
  CAM_COLOR_FX_BW           = V4L2_COLORFX_BW,             /**< [en] Black/white                 <BR> [jp] 白黒 */
  CAM_COLOR_FX_SEPIA        = V4L2_COLORFX_SEPIA,          /**< [en] Sepia                       <BR> [jp] セピア */
  CAM_COLOR_FX_NEGATIVE     = V4L2_COLORFX_NEGATIVE,       /**< [en] positive/negative inversion <BR> [jp] ネガ */
  CAM_COLOR_FX_EMBOSS       = V4L2_COLORFX_EMBOSS,         /**< [en] Emboss                      <BR> [jp] エンボス */
  CAM_COLOR_FX_SKETCH       = V4L2_COLORFX_SKETCH,         /**< [en] Sketch                      <BR> [jp] スケッチ */
  CAM_COLOR_FX_SKY_BLUE     = V4L2_COLORFX_SKY_BLUE,       /**< [en] Sky blue                    <BR> [jp] スカイブルー */
  CAM_COLOR_FX_GRASS_GREEN  = V4L2_COLORFX_GRASS_GREEN,    /**< [en] Grass green                 <BR> [jp] 草色 */
  CAM_COLOR_FX_SKIN_WHITEN  = V4L2_COLORFX_SKIN_WHITEN,    /**< [en] Skin whiten                 <BR> [jp] 美白 */
  CAM_COLOR_FX_VIVID        = V4L2_COLORFX_VIVID,          /**< [en] Vivid                       <BR> [jp] 鮮明 */
  CAM_COLOR_FX_AQUA         = V4L2_COLORFX_AQUA,           /**< [en] Aqua                        <BR> [jp] アクア */
  CAM_COLOR_FX_ART_FREEZE   = V4L2_COLORFX_ART_FREEZE,     /**< [en] Art freeze                  <BR> [jp] アート */
  CAM_COLOR_FX_SILHOUETTE   = V4L2_COLORFX_SILHOUETTE,     /**< [en] Silhouette                  <BR> [jp] シルエット */
  CAM_COLOR_FX_SOLARIZATION = V4L2_COLORFX_SOLARIZATION,   /**< [en] Solarization                <BR> [jp] ソラリゼーション */
  CAM_COLOR_FX_ANTIQUE      = V4L2_COLORFX_ANTIQUE,        /**< [en] Antique                     <BR> [jp] アンティーク */
  CAM_COLOR_FX_SET_CBCR     = V4L2_COLORFX_SET_CBCR,       /**< [en] Set CbCr                    <BR> [jp] */
  CAM_COLOR_FX_PASTEL       = V4L2_COLORFX_PASTEL,         /**< [en] Pastel                      <BR> [jp] パステル */
};


/**
 * @defgroup CAM_ISO_SENSITIVITY Camera ISO Sensitivity parameter definitions
 * @brief Camera ISO Sensitivity parameter definitions.
 * @{
 */
#define CAM_ISO_SENSITIVITY_25    (25000)     /**< [en] ISO Sensitivity 25   <BR> [jp] ISO感度 25 */
#define CAM_ISO_SENSITIVITY_32    (32000)     /**< [en] ISO Sensitivity 32   <BR> [jp] ISO感度 32 */
#define CAM_ISO_SENSITIVITY_40    (40000)     /**< [en] ISO Sensitivity 40   <BR> [jp] ISO感度 40 */
#define CAM_ISO_SENSITIVITY_50    (50000)     /**< [en] ISO Sensitivity 50   <BR> [jp] ISO感度 50 */
#define CAM_ISO_SENSITIVITY_64    (64000)     /**< [en] ISO Sensitivity 64   <BR> [jp] ISO感度 64 */
#define CAM_ISO_SENSITIVITY_80    (80000)     /**< [en] ISO Sensitivity 80   <BR> [jp] ISO感度 80 */
#define CAM_ISO_SENSITIVITY_100   (100000)    /**< [en] ISO Sensitivity 100  <BR> [jp] ISO感度 100 */
#define CAM_ISO_SENSITIVITY_125   (125000)    /**< [en] ISO Sensitivity 125  <BR> [jp] ISO感度 125 */
#define CAM_ISO_SENSITIVITY_160   (160000)    /**< [en] ISO Sensitivity 160  <BR> [jp] ISO感度 160 */
#define CAM_ISO_SENSITIVITY_200   (200000)    /**< [en] ISO Sensitivity 200  <BR> [jp] ISO感度 200 */
#define CAM_ISO_SENSITIVITY_250   (250000)    /**< [en] ISO Sensitivity 250  <BR> [jp] ISO感度 250 */
#define CAM_ISO_SENSITIVITY_320   (320000)    /**< [en] ISO Sensitivity 320  <BR> [jp] ISO感度 320 */
#define CAM_ISO_SENSITIVITY_400   (400000)    /**< [en] ISO Sensitivity 400  <BR> [jp] ISO感度 400 */
#define CAM_ISO_SENSITIVITY_500   (500000)    /**< [en] ISO Sensitivity 500  <BR> [jp] ISO感度 500 */
#define CAM_ISO_SENSITIVITY_640   (640000)    /**< [en] ISO Sensitivity 640  <BR> [jp] ISO感度 640 */
#define CAM_ISO_SENSITIVITY_800   (800000)    /**< [en] ISO Sensitivity 800  <BR> [jp] ISO感度 800 */
#define CAM_ISO_SENSITIVITY_1000  (1000000)   /**< [en] ISO Sensitivity 1000 <BR> [jp] ISO感度 1000 */
#define CAM_ISO_SENSITIVITY_1250  (1250000)   /**< [en] ISO Sensitivity 1250 <BR> [jp] ISO感度 1250 */
#define CAM_ISO_SENSITIVITY_1600  (1600000)   /**< [en] ISO Sensitivity 1600 <BR> [jp] ISO感度 1600 */
/** @} */

/**
 * @enum CAM_HDR_MODE
 * @brief [en] Camera HDR mode definitions. <BR>
 *        [ja] CameraのHDRモードの設定値
 * @{
 */
enum CAM_HDR_MODE {
  CAM_HDR_MODE_OFF  = 0, /**< HDR off */
  CAM_HDR_MODE_AUTO = 1, /**< HDR auto */
  CAM_HDR_MODE_ON   = 2, /**< HDR on */
};

/**
 * @enum CAM_VIDEO_FPS
 * @brief [en] Camera Video Framerate setting parameters.
 *        [ja] Cameraのフレームレート設定値
 */
enum CAM_VIDEO_FPS {
  CAM_VIDEO_FPS_NONE, /**< Non frame rate. This is for Still Capture */
  CAM_VIDEO_FPS_5,    /**< 5 FPS */
  CAM_VIDEO_FPS_6,    /**< 6 FPS */
  CAM_VIDEO_FPS_7_5,  /**< 7.5 FPS */
  CAM_VIDEO_FPS_15,   /**< 15 FPS */
  CAM_VIDEO_FPS_30,   /**< 30 FPS */
  CAM_VIDEO_FPS_60,   /**< 60 FPS */
  CAM_VIDEO_FPS_120,  /**< 120 FPS */
};

/** @brief [en] Camera Callback type definition. <BR> [jp] Cameraからのコールバック関数の型定義 */
typedef void (*camera_cb_t)(CamImage img);


/**
 * @class ImgBuff
 * @brief [en] Camera Image memory management class. This is internal class. <BR>
 *        [ja] Cameraのイメージメモリ管理用クラス。内部利用Class。
 */
class ImgBuff {
  static const int SPRESENSE_CAMIMAGE_MEM_ALIGN = 32;

  int ref_count;
  uint8_t *buff;
  int width;
  int height;
  int idx;
  bool is_queue;
  enum v4l2_buf_type buf_type;

  CAM_IMAGE_PIX_FMT pix_fmt;

  size_t buf_size;
  size_t actual_size;

  CameraClass *cam_ref;

  sem_t my_sem;

  ImgBuff();
  ImgBuff(enum v4l2_buf_type type, int w, int h, CAM_IMAGE_PIX_FMT fmt, int jpgbufsize_divisor, CameraClass *cam);
  ~ImgBuff();

  bool is_valid(){ return (buff != NULL); };

  void lock()  { if(buff!=NULL) sem_wait(&my_sem); };
  void unlock(){ if(buff!=NULL) sem_post(&my_sem); };

  void queued(bool q){ lock(); is_queue = q; unlock(); };
  bool is_queued(void){ bool ret; lock(); ret = is_queue; unlock(); return ret; };

  void incRef();
  bool decRef();

  bool generate_imgmem(size_t s);
  size_t calc_img_size(int w, int h, CAM_IMAGE_PIX_FMT fmt, int jpgbufsize_divisor);
  void update_actual_size(size_t sz);

  static void delete_inst(ImgBuff *buf);

  friend CameraClass;
  friend CamImage;
};


/**
 * @class CamImage
 * @brief [en] The class which is to control Image from Camera. <BR>
 *        [ja] Cameraから得られる画像データを利用するためのクラス。
 */
class CamImage {

private:
  ImgBuff *img_buff;

  CamImage(enum v4l2_buf_type type, int w, int h, CAM_IMAGE_PIX_FMT fmt, int jpgbufsize_divisor = 7, CameraClass *cam = NULL);
  void setActualSize(size_t sz) { img_buff->update_actual_size(sz); };
  void setPixFormat(CAM_IMAGE_PIX_FMT pix_fmt) { if(img_buff != NULL) img_buff->pix_fmt = pix_fmt; }
  void setIdx(int i){ if(img_buff != NULL) img_buff->idx = i; }
  bool isIdx(int i){ return (img_buff != NULL) ? (img_buff->idx == i) : false; }
  int getIdx(){ return (img_buff != NULL) ? img_buff->idx : -1; }
  int getType(){ return (img_buff != NULL) ? img_buff->buf_type : -1; }
  bool is_valid(){ return (img_buff != NULL); };

  bool check_hw_resize_param(int iw, int ih, int ow, int oh);
  bool check_resize_magnification(int in, int out);


public:
  /**
   * @brief Get Image Width
   * @details [en] Get Image pixel width (px). <BR>
   *          [ja] 画像データの横サイズを取得する。(ピクセル単位)
   * @return [en] Width of the Image.(px) <BR>
   *         [ja] 画像データの横サイズ。(ピクセル単位)
   */
  int getWidth(){ return (img_buff != NULL) ? img_buff->width : 0; }

  /**
   * @brief Get Image Height
   * @details [en] Get Image pixel height (px). <BR>
   *          [ja] 画像データの縦サイズを取得する。(ピクセル単位)
   * @return [en] Width of the Image.(px) <BR>
   *         [ja] 画像データの縦サイズ。(ピクセル単位)
   */
  int getHeight(){ return (img_buff != NULL) ? img_buff->height : 0; }

  /**
   * @brief Get Image memory address.
   * @details [en] Get Image memory address to access Image data directly. <BR>
   *          [ja] 画像データに直接アクセスするためのデータのメモリアドレスを取得する。
   * @return [en] Image memory address. <BR>
   *         [ja] 画像データのアドレス
   */
  uint8_t * getImgBuff() { return (img_buff != NULL) ? img_buff->buff : NULL; }

  /**
   * @brief Get Image Size.
   * @details [en] Get Image data size (bytes). <BR>
   *          [ja] イメージデータサイズを返す。(byte単位)
   * @return [en] Actual image size. (bytes). <BR>
   *         [jp] 実際のデータ長。(byte単位)
   */
  size_t getImgSize() { return (img_buff != NULL) ? img_buff->actual_size : 0; }

  /**
   * @brief Get image buffer size.
   * @details [en] Get image buffer size (bytes). <BR>
   *          [ja] 画像バッファサイズを返す。(byte単位)
   * @return [en] Image buffer size. (bytes). <BR>
   *         [jp] 画像バッファサイズ。(byte単位)
   */
  size_t getImgBuffSize() { return (img_buff != NULL) ? img_buff->buf_size : 0; }

  /**
   * @brief Get Image Pixcel format.
   * @details [en] Get Pixcel format of this Image. <BR>
   *          [ja] イメージデータのピクセルフォーマットを返す。
   * @return [en] Enum value of #CAM_IMAGE_PIX_FMT <BR>
   *         [jp] #CAM_IMAGE_PIX_FMT で定義されているピクセルフォーマット
   */
  CAM_IMAGE_PIX_FMT getPixFormat() { return (img_buff != NULL) ? img_buff->pix_fmt : CAM_IMAGE_PIX_FMT_NONE; }

  /**
   * @brief Constuctor of CamImage class
   * @details [en] Construct empty CamImage instance. <BR>
   *          [ja] 空のCamImageインスタンスを生成する。
   * @return [en] Empty CamImage instance <BR>
   *         [jp] 空のCamImageインスタンス
   */
  CamImage() : img_buff(NULL) {};

  /**
   * @brief Copy Constuctor of CamImage class
   * @details [en] Construct new CamImage class copied from inputted instance.
   *               Internaly, the image data buffer is not created, but this makes
   *               the reference counter of the buffer just incremented. <BR>
   *          [ja] 入力されたCamImageインスタンスのコピーのインスタンスを生成する。
   *               内部では、画像データバッファは新たに作成されず、参照カウンタが＋１される。
   * @return [en] Copied CamImage instance. <BR>
   *         [jp] コピーされたCamImageインスタンス
   */
  CamImage(const CamImage &obj /**< [en] Instance to copy. <BR> [ja] コピー元のインスタンス */);

  /**
   * @brief Assignment operator.
   * @details [en] This do 2 jobs. 1st. delete the old instance. 2nd. increment
   *               the reference counter of new assigned instance. <BR>
   *          [ja] このメソッドでは主に2つの処理が行われる。第1に古いインスタンス
   *               の削除。第2に新しく代入されたインスタンスの参照カウンタを＋１する。
   * @return [en] instance of assigned. <BR>
   *         [jp] 代入されるインスタンス
   */
  CamImage &operator=(const CamImage &obj /**< [en] Instance to be assigned. <BR> [ja] 代入対象インスタンス */);

  /**
   * @brief Convert Pixcelformat of the image.
   * @details [en] Convert own image's pixel format. Override Image data. So
   *               original image is discarded. If paramter is the same format
   *               as current, no error and no operation. <BR>
   *          [ja] ピクセルフォーマット変換を行う。画像データは上書きされ、元の
   *               ピクセルフォーマットの画像は破棄される。現在のフォーマットと
   *               同一のフォーマットが設定された場合、何も処理は行われず正常終了する。
   * @return [en] Error codes in #CamErr <BR>
   *         [jp] #CamErr で定義されているエラーコード
   */
  CamErr convertPixFormat(CAM_IMAGE_PIX_FMT to_fmt /**< [en] Pixcel format which is convert to. <BR> [ja] 変換するピクセルフォーマット */);

  /**
   * @brief Resize Image with HW 2D accelerator.
   * @details [en] Resize the image with 2D accelerator HW in CXD5602.
   *               Internaly, new image buffer is created, and the resized image is in it.
   *               After resized, CamImage instance of 1st argument stores it.
   *               If any error occured such as zero size case, this returns error code.
   *               This HW accelerator has limitation as below: <BR>
   *               - Minimum width and height is 12 pixels.
   *               - Maximum width is 768 pixels.
   *               - Maximum height is 1024 pixels.
   *               - Resizing magnification is 2^n or 1/2^n, and resized image size must be integer. <BR>
   *          [ja] CXD5602が持つ2Dアクセラレータを用いた画像のリサイズを行う。
   *               内部で新たにImage用のバッファを生成したうえで、第1引数に指定された
   *               CamImageインスタンスに結果を格納する。
   *               指定されたサイズがゼロの場合など、何らかのエラーが起きた場合、空の
   *               CamImageインスタンスを格納し、エラーコードを返す。
   *               このHWアクセラレータには、以下の仕様制限があります。<BR>
   *               イメージの幅、高さの最小ピクセル数は12ピクセル。
   *               イメージの幅の最大ピクセル数は768ピクセル。
   *               イメージの高さの最大ピクセル数は1024ピクセル。
   *               リサイズする場合の倍率は2^n倍もしくは1/2^nとなり、リサイズ後のサイズは整数になる必要がある。 <BR>
   * @return [en] Error codes in #CamErr <BR>
   *         [jp] #CamErr で定義されているエラーコード
   */
  CamErr resizeImageByHW(
    CamImage &img, /**< [en] Instance of CamImage with result of resizing. <BR> [ja] リサイズ後の新しいCamImageが格納されるインスタンス */
    int width, /**< [en] Width to resize  <BR> [ja] リサイズする画像の横サイズ */
    int height /**< [en] Height to resize <BR> [ja] リサイズする画像の縦サイズ */
  );


  /**
   * @brief Clip and resize Image with HW 2D accelerator.
   * @details [en] Clip and resize the image with 2D accelerator HW in CXD5602.
   *               First, clip the area specified by the arguments (#lefttop_x, #lefttop_y) - (#rightbottom_x, # rightbottom_y) for the original
   *               image and specify the clipped image with arguments (#width, # height) resize to the size you made.
   *               The resized image is stored in the CamImage instance specified as the first argument with new image buffer created internally.
   *               If any error occured such as zero size case, this returns error code.
   *               This HW accelerator has limitation for resizing as below: <BR>
   *               - Minimum width and height is 12 pixels.
   *               - Maximum width is 768 pixels.
   *               - Maximum height is 1024 pixels.
   *               - Resizing magnification is 2^n or 1/2^n, and resized image size must be integer. <BR>
   *          [ja] CXD5602が持つ2Dアクセラレータを用いた画像のクリッピング及びリサイズを行う。
   *               まず、元画像に対して、引数 (#lefttop_x, #lefttop_y) - (#rightbottom_x, #rightbottom_y) で指定された領域をクリップし、
   *               クリップされた画像に対して引数 (#width, #height)で指定されたサイズにリサイズを行う。
   *               リサイズ後の画像は、内部で新たにImage用のバッファを生成したうえで第1引数に指定されたCamImageインスタンスに結果を格納する。
   *               指定されたサイズがゼロの場合など、何らかのエラーが起きた場合、空のCamImageインスタンスを格納し、エラーコードを返す。
   *               なお、このHWアクセラレータには、リサイズ動作に関して以下の仕様制限があります。<BR>
   *               　　イメージの幅、高さの最小ピクセル数は12ピクセル。<BR>
   *               　　イメージの幅の最大ピクセル数は768ピクセル。<BR>
   *               　　イメージの高さの最大ピクセル数は1024ピクセル。<BR>
   *               　　リサイズする場合の倍率は2^n倍もしくは1/2^nとなり、リサイズ後のサイズは整数になる必要がある。 <BR>
   * @return [en] Error codes in #CamErr <BR>
   *         [jp] #CamErr で定義されているエラーコード
   */
  CamErr clipAndResizeImageByHW(
    CamImage &img,     /**< [en] Instance of CamImage with result of resizing. <BR> [ja] リサイズ後の新しいCamImageが格納されるインスタンス */
    int lefttop_x,     /**< [en] Left top X coodinate in original image for clipping. <BR> [ja] 元画像に対して、クリップする左上のX座標 */
    int lefttop_y,     /**< [en] Left top Y coodinate in original image for clipping. <BR> [ja] 元画像に対して、クリップする左上のY座標 */
    int rightbottom_x, /**< [en] Right bottom X coodinate in original image for clipping. <BR> [ja] 元画像に対して、クリップする左上のX座標 */
    int rightbottom_y, /**< [en] Right bottom Y coodinate in original image for clipping. <BR> [ja] 元画像に対して、クリップする左上のY座標 */
    int width,         /**< [en] Width to resize from clipping image  <BR> [ja] クリップされた画像に対して、リサイズする画像の横サイズ */
    int height         /**< [en] Height to resize from clipping image <BR> [ja] クリップされた画像に対して、リサイズする画像の縦サイズ */
  );


  /**
   * @brief Check valid image data.
   * @details [en] Confirm availability of this image instance.<BR>
   *          [ja] 利用可能な画像データかどうかをチェックする。
   * @return [en] true if the instance has correct image.<BR>
   *         [ja] 利用可能な画像データであればtrueが返る。
   */
  bool isAvailable(void);

  /**
   * @brief Destructor of CamImage.
   * @details [en] Destroy CamImage <BR>
   *          [ja] CamImageインスタンスの破棄を行う。
   */
  ~CamImage();


private:

  friend CameraClass;
};



/**
 * @class CameraClass
 * @brief [en] The class to control Spresense Camera. <BR>
 *        [ja] SpresenseのCamera機能を制御するためのクラス。
 */
class CameraClass {
private:
  friend ImgBuff;

  int video_fd;
  int video_init_stat;
  int video_buf_num;
  CAM_IMAGE_PIX_FMT video_pix_fmt;
  CAM_IMAGE_PIX_FMT still_pix_fmt;
  CamImage **video_imgs;
  CamImage *still_img;
  static CameraClass *instance;
  volatile bool loop_dqbuf_en;

  sem_t video_cb_access_sem;
  camera_cb_t video_cb;

  CameraClass(const char *path);

  CamErr convert_errno2camerr(int err);
  CamErr set_frame_parameters( enum v4l2_buf_type type, int video_width, int video_height, int buf_num, CAM_IMAGE_PIX_FMT video_fmt );
  CamErr create_videobuff(int w, int h, int buff_num, CAM_IMAGE_PIX_FMT fmt, int jpgbufsize_divisor);
  void delete_videobuff();
  CamErr enqueue_video_buffs();
  CamErr enqueue_video_buff(CamImage *img);
  bool is_device_ready();
  CamErr set_video_frame_rate(CAM_VIDEO_FPS fps);
  CamErr set_ext_ctrls(uint16_t ctl_cls, uint16_t cid, int32_t value);
  int32_t get_ext_ctrls(uint16_t ctl_cls, uint16_t cid);
  CamErr create_stillbuff(int w, int h, CAM_IMAGE_PIX_FMT fmt, int jpgbufsize_divisor);
  CamErr create_dq_thread();
  void   delete_dq_thread();

  void lock_video_cb()  { sem_wait(&video_cb_access_sem); };
  void unlock_video_cb(){ sem_post(&video_cb_access_sem); };

  pthread_t frame_tid;
  static void frame_handle_thread(void *);
  static const int CAM_FRAME_THREAD_STACK_SIZE = 2048;
  static const int CAM_FRAME_THREAD_STACK_PRIO = 101;

  mqd_t frame_exchange_mq;
  static const int CAM_FRAME_MQ_SIZE = 1;

  pthread_t dq_tid;
  static void dqbuf_thread(void *);
  static const int CAM_DQ_THREAD_STACK_SIZE = 1024;
  static const int CAM_DQ_THREAD_STACK_PRIO = 102;

  int ioctl_dequeue_stream_buf(struct v4l2_buffer *buf, uint16_t type);
  CamImage *search_vimg(int index);
  void release_buf(ImgBuff *buf);

public:

  /**
   * @brief Destruct CameraClass instance.
   */
  ~CameraClass();

  /**
   * @brief Get the Camera instance.
   * @details [en] Get the Camera instance. Usually the sketch should not
   *               use this method. Please use defined global variable of
   *               "theCamera". <BR>
   *          [ja] CameraClassインスタンスを取得するメソッド。通常、スケッチでは
   *               このメソッドは使わないようにしてください。CameraClassのインス
   *               タンスは、グローバル変数として、"theCamera"を定義している
   *               のでそちらを使ってくだささい。
   * @return [en] Instance of CameraClass. <BR>
   *         [ja] CameraClassインスタンス
   */
  static CameraClass getInstance();

  /**
   * @brief Get the file descriptor of camera device.
   * @details [en] Get the file descriptor of camera device.<BR>
   *          [ja] カメラデバイスのファイルディスクリプタを取得する。
   * @return [en] The file descriptor of camera device.
   *              Return CAM_ERR_NO_DEVICE if begin() methods is not executed.<BR>
   *         [ja] カメラデバイスのファイルディスクリプタ。
   *              begin()メソッドが実行されていない場合は、CAM_ERR_NO_DEVICEを返す。
   */
  int getFd() { return (video_fd < 0) ? CAM_ERR_NO_DEVICE : video_fd; }

  /**
   * @brief Initialize CameraClass instance.
   * @details [en] Initialize CameraClass Instance. This method must be called before
   *               use any other methods. With initialization, image buffers
   *               which is used as video buffer to get is generated. <BR>
   *          [ja] CameraClassインスタンスの初期化を行う。このメソッドはほかのメソ
   *               ッドを利用する前に必ず呼び出す必要がある。この初期化に伴っ
   *               て、Videoストリームとして利用するVideoバッファも確保される。
   * @return [en] Error code defined as #CamErr. <BR>
   *         [ja] #CamErr で定義されているエラーコード
   */
  CamErr begin(
    int buff_num = 1,                       /**< [en] Number of video stream image buffer.(Default : 1)                          <BR> [ja] Videoストリームで利用するバッファの数 (デフォルト 1枚) */
    CAM_VIDEO_FPS fps = CAM_VIDEO_FPS_30,   /**< [en] Frame rate of video stream. Choose one in #CAM_VIDEO_FPS (Default : 30FPS) <BR> [ja] Videoストリームのフレームレート。 #CAM_VIDEO_FPS の中から選択 (デフォルト 30FPS) */
    int video_width   = CAM_IMGSIZE_QVGA_H, /**< [en] Image buffer width of video stream.(px)(Default : QVGA)                    <BR> [ja] Videoストリーム画像の横サイズ (単位ピクセル)(デフォルト QVGA) */
    int video_height  = CAM_IMGSIZE_QVGA_V, /**< [en] Image buffer height of video stream.(px)(Default : QVGA)                   <BR> [ja] Videoストリーム画像の縦サイズ (単位ピクセル)(デフォルト QVGA) */
    CAM_IMAGE_PIX_FMT video_fmt = CAM_IMAGE_PIX_FMT_YUV422, /**< [en] Video stream image buffer pixel format.(Default : YUV422) <BR> [ja] Videoストリームで利用するバッファのピクセルフォーマット (デフォルト YUV422) */
    int jpgbufsize_divisor = 7              /**< [en] The divisor of JPEG buffer size formula. buffer size = video_width * video_height * 2 / jpgbufsize_divisor (Default : 7) <BR>
                                             * [ja] JPEG用バッファサイズ計算式における除数。バッファサイズ = video_width * video_height * 2 / jpgbufsize_divisor (デフォルト : 7) */
  );

  /**
   * @brief Start / Stop Video Stream
   * @details [en] Start / Stop video stream. After call this method with enable,
   *               video stream from Spresense Camera starts. The video image
   *               from Camera can be captured by callback of #camera_cb_t <BR>
   *          [ja] Spresense CameraのVideoストリームを開始/停止する。このメソッド
   *               がenableで呼び出されるとSpresense CameraのVideoストリームが動き
   *               出す。Video画像は #camera_cb_t のコールバック関数の呼び出しによ
   *               り取得できる。
   * @return [en] Error code defined as #CamErr. <BR>
   *         [ja] #CamErr で定義されているエラーコード
   */
  CamErr startStreaming(
    bool enable,          /**< [en] Start or Stop streaming. (ture : start, false : stop) <BR> [ja] ストリームの開始/停止 (true : 開始、false : 停止) */
    camera_cb_t cb = NULL /**< [en] Callback function to capture the video image.         <BR> [ja] Video画像を取得するためのコールバック関数 */
  );

  /**
   * @brief Control Auto White Balance
   * @details [en] Start / Stop Auto White Balance. <BR>
   *          [ja] 自動ホワイトバランス調整の開始/停止の制御を行う
   * @return [en] Error code defined as #CamErr. <BR>
   *         [ja] #CamErr で定義されているエラーコード
   */
  CamErr setAutoWhiteBalance(bool enable /**< [en] Start or Stop Auto White Balance. (true : start, false : stop) <BR> [ja] 自動ホワイトバランス調整の開始/停止 (true : 開始、false : 停止) */);

  /**
   * @brief Control Auto Exposure
   * @details [en] Start / Stop Auto Exposure <BR>
   *          [ja] 自動露光調整の開始/停止の制御を行う
   * @return [en] Error code defined as #CamErr. <BR>
   *         [ja] #CamErr で定義されているエラーコード
   */
  CamErr setAutoExposure(bool enable /**< [en] Start or Stop Auto Exposure. (true : start, false : stop) <BR> [ja] 自動露光調整の開始/停止 (true : 開始、false : 停止) */);

  /**
   * @brief Set exposure Time
   * @details [en] Set exposure time in 100usec units.  <BR>
   *          [ja] 露光時間(100usec単位)を設定する。
   *
   * @param  exposure_time [en] Exposure time in 100 usec units. ex) 10000 is one second. <BR>
   *                       [ja] 露光時間(100usec単位)。 例) 10000 = 1秒
   * @return [en] Error code defined as #CamErr. <BR>
   *         [ja] #CamErrで定義されているエラーコード。
   */
  CamErr setAbsoluteExposure(int32_t exposure_time);

  /**
   * @brief Get exposure Time
   * @details [en] Get exposure time in 100usec units.  <BR>
   *          [ja] 露光時間(100usec単位)を取得する。
   *
   * @return [en] Exposure time in 100usec units or error code defined as #CamErr. <BR>
   *         [ja] 露光時間(100usec単位) もしくは、#CamErrで定義されているエラーコード。
   */
  int32_t getAbsoluteExposure(void);

  /**
   * @brief Control Auto ISO Sensitivity (WIll obsolete after v1.2.0)
   * @details [en] Start / Stop Auto ISO Sensitivity <BR>
   *          [ja] 自動ISO感度調整の開始/停止の制御を行う
   * @return [en] Error code defined as #CamErr. <BR>
   *         [ja] #CamErr で定義されているエラーコード
   */
  CamErr setAutoISOSensitive(bool enable /**< [en] Start or Stop Auto ISO Sensitivity. (true : start, false : stop) <BR> [ja] 自動ISO感度調整の開始/停止 (true : 開始、false : 停止) */);

  /**
   * @brief Control Auto ISO Sensitivity
   * @details [en] Start / Stop Auto ISO Sensitivity <BR>
   *          [ja] 自動ISO感度調整の開始/停止の制御を行う
   * @return [en] Error code defined as #CamErr. <BR>
   *         [ja] #CamErr で定義されているエラーコード
   */
  CamErr setAutoISOSensitivity(bool enable /**< [en] Start or Stop Auto ISO Sensitivity. (true : start, false : stop) <BR> [ja] 自動ISO感度調整の開始/停止 (true : 開始、false : 停止) */);

  /**
   * @brief Set ISO Sensivity value.
   * @details [en] Set ISO Sensitivity value. This value is available in case of
   *               Auto ISO Seisitivity stopped. If re-enable Auto ISO Sensitivity,
   *               set value is disapired, so the value should set again. <BR>
   *          [ja] ISO感度設定。この値は自動ISO感度調整機能をOFFした場合にのみ有効。
   *               この値は、再度自動ISO感度調整を有効にした場合、破棄される。その
   *               場合、再設定が必要になる。
   * @return [en] Error code defined as #CamErr. <BR>
   *         [ja] #CamErr で定義されているエラーコード
   */
  CamErr setISOSensitivity(int iso_sense /**< [en] ISO Sensitivity value. Use macros named @ref CAM_ISO_SENSITIVITY  <BR> [ja] ISO感度値。 @ref CAM_ISO_SENSITIVITY と定義されたマクロから選択する */);

  /**
   * @brief Get ISO Sensivity value.
   * @details [en] Get ISO Sensitivity value.
   *          [ja] ISO感度を取得する。
   * @return [en] ISO Sensitivity value or Error code defined as #CamErr. <BR>
   *         [ja] ISO感度もしくは、#CamErr で定義されているエラーコード
   */
  int getISOSensitivity(void);

  /**
   * @brief Set Auto White Balance mode.
   * @details [en] Set Auto White Balance mode. <BR>
   *          [ja] 自動ホワイトバランス調整モードの設定。
   * @return [en] Error code defined as #CamErr. <BR>
   *         [ja] #CamErr で定義されているエラーコード
   */
  CamErr setAutoWhiteBalanceMode(CAM_WHITE_BALANCE wb /**< [en] White Balance mode. Choose one from #CAM_WHITE_BALANCE <BR> [ja] ホワイトバランスモード設定。 #CAM_WHITE_BALANCE より設定値を選択する */);

#if 0 /* To Be Supported */
  /*
   * @brief Set Scene Mode.
   * @details [en] Set Scene Mode. <BR>
   *          [ja] シーンモードの設定。
   * @return [en] Error code defined as #CamErr. <BR>
   *         [ja] #CamErr で定義されているエラーコード
   */
  CamErr setSceneMode(CAM_SCENE_MODE mode /**< [en] Scene mode parameter. Choose one from #CAM_SCENE_MODE <BR> [ja] シーンモード設定値。 #CAM_SCENE_MODE から選択する */);
#endif

  /**
   * @brief Set Color Effect.
   * @details [en] Set Color Effect. <BR>
   *          [ja] 色効果の設定
   * @return [en] Error code defined as #CamErr. <BR>
   *         [ja] #CamErr で定義されているエラーコード
   */
  CamErr setColorEffect(CAM_COLOR_FX effect /**< [en] Color effect. Choose one from #CAM_COLOR_FX <BR> [ja] 色効果設定値。 #CAM_COLOR_FX から選択する */ );

  /**
   * @brief Set HDR mode.
   * @details [en] Set HDR mode. The default mode is HDR auto(CAM_HDR_MODE_AUTO) <BR>
   *          [ja] HDRモードを設定する。デフォルトはHDR auto(CAM_HDR_MODE_AUTO)。
   * @return [en] Error code defined as #CamErr. <BR>
   *         [ja] #CamErr で定義されているエラーコード
   */
  CamErr setHDR(CAM_HDR_MODE mode /**< [en] HDR mode value. Choose one from #CAM_HDR_MODE <BR> [ja] HDRモード。 #CAM_HDR_MODE から選択する。 */ );

  /**
   * @brief Get HDR mode.
   * @details [en] Get HDR mode. <BR>
   *          [ja] HDRモードを取得する。
   * @return [en] HDR mode defined as #CAM_HDR_MODE. <BR>
   *         [ja] #CAM_HDR_MODE で定義されているHDRモード。
   */
  CAM_HDR_MODE getHDR(void);

  /**
   * @brief Set JPEG quality
   * @details [en] Set JPEG quality.  <BR>
   *          [ja] JPEG品質を設定する。
   *
   * @param  quality [en] JPEG quality(1-100). In ISX019 case, 1-4 are rounded up to 10, and 5-100 are rounded to the nearest 10. <BR>
   *                 [ja] JPEG品質(1-100)。ISX019を使用する場合、1-4は10に切り上げ、5-100は1の位を四捨五入し10の倍数に丸め込まれる。
   * @return [en] Error code defined as #CamErr. <BR>
   *         [ja] #CamErrで定義されているエラーコード。
   */
  CamErr setJPEGQuality(int quality);

  /**
   * @brief Get JPEG quality
   * @details [en] Get JPEG quality.  <BR>
   *          [ja] JPEG品質を取得する。
   *
   * @return [en] JPEG quality or error code defined as #CamErr. <BR>
   *         [ja] JPEG品質 もしくは、#CamErrで定義されているエラーコード。
   */
  int getJPEGQuality(void);

  /**
   * @brief Get frame interval
   * @details [en] Get frame interval in 100usec units.  <BR>
   *          [ja] フレーム間隔(100usec単位)を取得する。
   *
   * @return [en] Frame interval in 100usec units or error code defined as #CamErr. <BR>
   *         [ja] フレーム間隔(100usec単位) もしくは、#CamErrで定義されているエラーコード。
   */
  int getFrameInterval(void);

  /**
   * @brief Set Still Picture Image format parameters.
   * @details [en] Set Still Picture Image format. <BR>
   *          [ja] 静止画写真の画像フォーマット設定。
   * @return [en] Error code defined as #CamErr. <BR>
   *         [ja] #CamErr で定義されているエラーコード
   */
  CamErr setStillPictureImageFormat(
    int img_width,                                    /**< [en] Image width of Still picture.(px)   <BR> [ja] 静止画写真の横サイズ (単位ピクセル) */
    int img_height,                                   /**< [en] Image height of Still picture.(px)  <BR> [ja] 静止画写真の縦サイズ (単位ピクセル) */
    CAM_IMAGE_PIX_FMT img_fmt = CAM_IMAGE_PIX_FMT_JPG,/**< [en] Image pixel format. (Default JPEG) <BR> [ja] 静止画ピクセルフォーマット (デフォルト JPEG) */
    int jpgbufsize_divisor = 7              /**< [en] The divisor of JPEG buffer size formula. buffer size = img_width * img_height * 2 / jpgbufsize_divisor (Default : 7) <BR>
                                             * [ja] JPEG用バッファサイズ計算式における除数。バッファサイズ = img_width * img_height * 2 / jpgbufsize_divisor (デフォルト : 7) */
  );

  /**
   * @brief Take picture.
   * @details [en] Take picture with picture parameters which is set on #setStillPictureImageFormat() . <BR>
   *          [ja] 写真撮影。 #setStillPictureImageFormat() で設定した写真フォーマットに従って、写真を撮る。
   * @return [en] Taken picture. If any error occured, the result value has empty object. <BR>
   *         [ja] 撮影された写真イメージ。もし何らかのエラーが発生した場合、空のCamImageオブジェクトが返される。
   */
  CamImage takePicture();

  /**
   * @brief Get camera device type.
   * @details [en] Get camera device type which is being used. <BR>
   *          [ja] 使用されているカメラデバイスの種類を取得する。
   * @return [en] Camera device type which is being used. <BR>
   *         [ja] 使用されているカメラデバイスの種類。
   */

  CAM_DEVICE_TYPE getDeviceType();

  /**
   * @brief De-initialize Spresense Camera
   * @details [en] De-initialize Spresense Camera. This method cancel everything of
   *               Camera. If you want to use the Camera again, you should call
   *               #CameraClass::begin() method again. <BR>
   *          [ja] Spresense Cameraの終了処理を行う。このメソッドが呼び出されると、
   *               Cameraで行っていたのすべてがキャンセルされる。再度Cameraを利用し
   *               たい場合には #CameraClass::begin() メソッドを再度呼び出す必要がある。
   */
  void end();
};

extern CameraClass theCamera;

/** @} camera */

#endif // __SPRESENSE_CAMERA_CLASS_H__

