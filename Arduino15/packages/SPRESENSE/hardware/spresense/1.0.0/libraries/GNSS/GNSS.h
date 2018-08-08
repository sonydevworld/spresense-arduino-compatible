/*
 *  GNSS.h - GNSS include file for the Spresense SDK
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

#ifndef Gnss_h
#define Gnss_h

#include <Stream.h>

/**
 * @file GNSS.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino GNSS library
 * @details It is a library for controlling the GNSS built in Spresense
 *          and acquiring positioning information. This library is available
 *          in the Arduino environment.
 */

/**
 * @enum SpFixMode
 * @brief status of pos fix
 */
enum SpFixMode {
    FixInvalid = 1,
    Fix2D,
    Fix3D
};

/**
 * @enum SpStartMode
 * @brief Mode to set to GNSS at the start of positioning
 *
 * @details Depending on the reception status of the GNSS signal,
 *          the positioning FIX is fast in the order of HOT, WARM, COLD.
 *          In this library HOT is chosen as default.
 */
enum SpStartMode {
    COLD_START, /**< Cold start */
    WARM_START, /**< Warm start */
    HOT_START   /**< Hot start */
};

/**
 * @enum SpPrintLevel
 * @brief Set the debug log output level
 */
enum SpPrintLevel {
    PrintNone = 0,
    PrintError,
    PrintWarning,
    PrintInfo,
};

/**
 * @enum SpPvtType
 * @brief Set the GNSS positioning type
 */
enum SpPvtType {
    SpPvtTypeNone = 0, /**< Positioning data none */
    SpPvtTypeGnss,     /**< by GNSS */
    SpPvtTypeReserv,   /**< reserv */
    SpPvtTypeUsers     /**< API setting */
};

/**
 * @class SpGnssTime
 * @brief Time acquired from the satellite at the time of positioning
 *
 * @details Since each member is public, it can be accessed directly.\n
 *          'sec' represents seconds in the range 0 to 59,\n
 *          'usec' represents microseconds ranging from 0 to 999,999,\n
 *          and all subseconds are included in 'usec'.
 */
class SpGnssTime {
public:
  unsigned short year;  /**< Year(1980..) */
  unsigned char month;  /**< Month(1..12) */
  unsigned char day;    /**< Day(1..31) */
  unsigned char hour;   /**< Hour(0..23) */
  unsigned char minute; /**< Minute(0..59) */
  unsigned char sec;    /**< Second(0..59) */
  unsigned long usec;   /**< Microsecond(0..999999) */
};

/**
 * @class SpSatellite
 * @brief Satellite infomation using positioning
 *
 * @details This is debug information when there is a problem with positioning.
 */
class SpSatellite {
public:
  unsigned short type;      /**< Satellite type of GPS or Glonass */
  unsigned char  svid;      /**< Satellite ID */
  unsigned char  elevation; /**< Elevation of satellite [degree] */
  unsigned char  azimuth;   /**< Azimuth of satellite [degree]; Clockwise from the north */
  float sigLevel;           /**< C/N [dBHz] */
};

/**
 * @class SpNavData
 * @brief GNSS positioning data
 *
 * @details Store the positioning result in this object.
 */
class SpNavData {
public:
    SpGnssTime time;    /**< Time when this position data was updated */
    unsigned char type;           /**< Position type; 0:Invalid, 1:GNSS, 2:reserv, 3:user set, 4:previous */
    unsigned char numSatellites;  /**< Number of visible satellites */
    unsigned char posFixMode;     /**< FIX mode, 0:Invalid, 1:2D FIX, 2:3D FIX */
    unsigned char posDataExist;   /**< Is position data existed, 0:none, 1:exist */
    unsigned char numSatellitesCalcPos; /**< Number of satellites to calculate the position */
    unsigned short satelliteType;       /**< using sv system, bit field; bit0:GPS, bit1:GLONASS */
    unsigned short posSatelliteType;    /**< using sv system, bit field; bit0:GPS, bit1:GLONASS */
    double  latitude;   /**< Latitude [degree] */
    double  longitude;  /**< Longitude [degree] */
    double  altitude;   /**< Altitude [degree] */
    float   velocity;   /**< Velocity [m/s] */
    float   direction;  /**< Direction [degree] */
    float   pdop;       /**< Position DOP [-] */
    float   hdop;       /**< Horizontal DOP [-] */
    float   vdop;       /**< Vertical DOP [-] */
    float   tdop;       /**< Time DOP [-] */
    SpSatellite satellite[24];  /**< satellite data array */

    /**
     * @brief Check if the specified satellite is GPS
     * @details If the satellite of the element of array index is GPS, 1 is returned.
     * @param [in] index Array number of the satellite array
     * @return 1 GPS, 0 not GPS
     */
    int isSatelliteTypeGps(unsigned long index);

    /**
     * @brief Check if the specified satellite is Glonass
     * @details If the satellite of the element of array index is Glonass, 1 is returned.
     * @param [in] index Array number of the satellite array
     * @return 1 Glonass, 0 not Glonass
     */
    int isSatelliteTypeGlonass(unsigned long index);

    /**
     * @brief Get satellite ID(SVID)
     * @details Specify the element number of the satellite and return the SatelliteId value stored in the array.
     * @param [in] index Array number of the satellite array
     * @return Value of array SatelliteId
     */
    unsigned char getSatelliteId(unsigned long index);

    /**
     * @brief Get satellite elevation
     * @details Specify the element number of the satellite and return the SatelliteElevation value stored in the array.
     * @param [in] index Array number of the satellite array
     * @return Value of array SatelliteElevation
     */
    unsigned char getSatelliteElevation(unsigned long index);

    /**
     * @brief Get satellite azimuth
     * @details Specify the element number of the satellite and return the SatelliteAzimuth value stored in the array.
     * @param [in] index Array number of the satellite array
     * @return Value of array SatelliteAzimuth
     */
    unsigned char getSatelliteAzimuth(unsigned long index);

    /**
     * @brief Get satellite signal level(C/N)
     * @dtails Specify the element number of the satellite and return the SignalLevel value stored in the array.
     * @param [in] index Array number of the satellite array
     * @return Value of array SatelliteSignalLevel
     */
    float getSatelliteSignalLevel(unsigned long index);
};

/**
 * @class SpGnss
 * @brief GNSS controller
 *
 * @details You can control GNSS devices by operating SpGnss objects instantiated in your app.
 */
class SpGnss
{
public:
    friend SpNavData;

    /**
     * @brief Create SpGnss object
     */
    SpGnss();

    /**
     * @brief Destroy SpGnss object
     */
    ~SpGnss();

    /**
     * @brief Activate GNSS device
     * @details Power on GNSS hardware block, and change to the state where parameter
     *          setting and positioning start can be performed.
     * @param [in] debugOut debug out stream instead of Serial
     * @return 0 if success, -1 if failure
     */
    int begin(void);
    int begin(Stream& debugOut) { DebugOut = debugOut; return begin(); }

    /**
     * @brief Inactivate GNSS device
     * @details Power off GNSS hardware block.
     * @return 0 if success, -1 if failure
     */
    int end(void);

    /**
     * @brief Start positioning
     * @param [in] mode set start mode(SpStartMode)
     * @return 0 if success, -1 if failure
     *
     * If not specified mode, force set to hot start mode.
     */
    int start(SpStartMode mode = HOT_START);

    /**
     * @brief Stop positioning
     * @details Power off most of the hardware and change it to the idling state.
     * @return 0 if success, -1 if failure
     */
    int stop(void);

    /**
     * @brief Check position infomation is updated and return immediately
     * @return 1 enable, 0 disable
     *
     * Returns 1 if position information is updated.
     */
    int isUpdate(void);

    /**
     * @brief Wait for position information to be updated
     * @details Calling this function will block until GNSS positioning information is updated.
     * @param [in] timeout timeout of waiting
     * @return 1 enable, 0 disable
     *
     * Returns 1 if position information is updated.
     * If not specified timeout, wait forever.
     */
    int waitUpdate(long timeout = -1);

    /**
     * @brief Get updated positioning information from GNSS
     * @details This function copies the updated information to the specified navData.
     *          Instantiate the navData object specified by argument in app,
     * and specified navData to argument by the pointer address.
     * @param [out] navData includes latest positioning information
     * @return none
     *
     * Give NavData the address of the object instantiated in your app as an argument.
     * The latest position information when the function is called is stored into this object.
     */
    void getNavData(SpNavData *navData);

    /*
     * Get position data size.
     */
    unsigned long getPositionDataSize(void);

    /*
     * Get position data.
     */
    unsigned long getPositionData(char *pBinaryBuffer);

    /**
     * @brief Set the current position for hot start
     * @details In order to perform Hot start, set the approximate current position.
     *          The position held inside the GNSS device is overwritten.
     * @param [in] latitude Latitude of current position
     * @param [in] longitude Longitude of current position
     * @param [in] altitude Altitude of current position
     * @return 0 if success, -1 if failure
     */
    int setPosition(double latitude, double longitude, double altitude = 0);

    /**
     * @brief Set the current time for hot start
     * @details In order to perform Hot start, set the approximate current time.
     *          The time held inside the GNSS device is overwritten.
     * @param [in] time current time(SpGnssTime)
     * @return 0 if success, -1 if failure
     */
    int setTime(SpGnssTime *time);

    /**
     * @brief Set the pos interval time
     * @details Set interval of POS operation.
     * @param [in] Interval time[sec]
     * @return 0 if success, -1 if failure
     */
    int setInterval(long interval = 1);

    /**
     * @brief Returns whether GPS is used as satellite system
     * @details Make sure GPS is used for the satellite system used for positioning.
     * @return 1 use, 0 unuse
     */
    int isGps(void);

    /**
     * @brief Use GPS for positioning
     * @details Add GPS to the satellite system using for positioning.
     * @return 0 if success, -1 if failure
     */
    int useGps(void);

    /**
     * @brief Unuse GPS for positioning
     * details Remove the GPS from the satellite system used for positioning.
     * @return 0 if success, -1 if failure
     */
    int unuseGps(void);

    /**
     * @brief Returns whether Glonass is used as satellite system
     * @details Make sure Glonass is used for the satellite system used for positioning.
     * @return 1 use, 0 unuse
     */
    int isGlonass(void);

    /**
     * @brief Use Glonass for positioning
     * @details Add Glonass to the satellite system using for positioning.
     * @return 0 if success, -1 if failure
     */
    int useGlonass(void);

    /**
     * @brief Unuse Glonass for positioning
     * @details Remove the Glonass from the satellite system used for positioning.
     * @return 0 if success, -1 if failure
     */
    int unuseGlonass(void);

    /**
     * @brief Set debug mode
     * @details Print debug messages about GNSS controling and positioning if not set 0 to argument.
     * @param [in] level debug mode
     * @return none
     */
    void setDebugMode(SpPrintLevel level);

    /**
     * @brief Save the data stored in the backup RAM to Flash
     * @return 0 if success, -1 if failure
     */
    int saveEphemeris(void);

private:
    int fd_;                /* file descriptor */
    long SatelliteSystem;    /* satellite type */
    SpNavData NavData;    /* copy pos data */

    /* Debug Print */
    static Stream& DebugOut;
    static SpPrintLevel DebugPrintLevel;

    inline static void printMessage(SpPrintLevel level, const char* str)
    {
        if (level <= DebugPrintLevel)
            DebugOut.print(str);
    }
};

#endif // Gnss_h
