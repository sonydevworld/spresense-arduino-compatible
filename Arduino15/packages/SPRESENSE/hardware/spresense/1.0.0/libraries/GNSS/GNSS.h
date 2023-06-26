/*
 *  GNSS.h - GNSS include file for the Spresense SDK
 *  Copyright 2018, 2023 Sony Semiconductor Solutions Corporation
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
#include <GNSSPositionData.h>

/**
 * @file GNSS.h
 * @author Sony Semiconductor Solutions Corporation
 * @brief Spresense Arduino GNSS library
 * @details It is a library for controlling the GNSS built in Spresense
 *          and acquiring positioning information. This library is available
 *          in the Arduino environment.
 */

/**
 * @defgroup gnss GNSS Library API
 * @brief API for using GNSS
 * @{
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
 * @enum SpSatelliteType
 * @brief Satellite system type
 */
enum SpSatelliteType {
    GPS       = (1U << 0),
    GLONASS   = (1U << 1),
    SBAS      = (1U << 2),
    QZ_L1CA   = (1U << 3),
    QZ_L1S    = (1U << 5),
    BEIDOU    = (1U << 6),
    GALILEO   = (1U << 7),
    UNKNOWN   = 0,
};

/**
 * @enum SpIntervalFreq
 * @brief Interval frequency
 */
enum SpIntervalFreq {
    SpInterval_10Hz = 100,
    SpInterval_8Hz  = 125,
    SpInterval_5Hz  = 200,
    SpInterval_4Hz  = 250,
    SpInterval_2Hz  = 500,
    SpInterval_1Hz  = 1000,
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
 * @brief Satellite information using positioning
 *
 * @details This is debug information when there is a problem with positioning.
 */
class SpSatellite {
public:
  unsigned short type;      /**< Satellite type of GPS, Glonass, QZSS/Michibiki;
                                 w/ positioning augmentation of SBAS or QZSS L1S */
  unsigned char  svid;      /**< Satellite ID */
  unsigned char  elevation; /**< Elevation of satellite [degree] */
  signed short   azimuth;   /**< Azimuth of satellite [degree]; Clockwise from the north */
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
    unsigned char posFixMode;     /**< FIX mode, 1:Invalid, 2:2D FIX, 3:3D FIX */
    unsigned char posDataExist;   /**< Is position data existed, 0:none, 1:exist */
    unsigned char numSatellitesCalcPos; /**< Number of satellites to calculate the position */
    unsigned short satelliteType;       /**< using sv system, bit field; bit0:GPS, bit1:GLONASS */
    unsigned short posSatelliteType;    /**< using sv system, bit field; bit0:GPS, bit1:GLONASS */
    double  latitude;   /**< Latitude [degree] */
    double  longitude;  /**< Longitude [degree] */
    double  altitude;   /**< Altitude [meter] */
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
     * @param [in] sattype Type of satellite system
     * @return 1 match, 0 unmatch
     */
    int isSatelliteType(unsigned long index, SpSatelliteType sattype);

    /**
     * @brief [Obsolete] Check if the specified satellite is GPS
     * @details This function is obsolete. Replace with #isSatelliteType.
     */
    int isSatelliteTypeGps(unsigned long index) { return isSatelliteType(index, GPS); }

    /**
     * @brief [Obsolete] Check if the specified satellite is Glonass
     * @details This function is obsolete. Replace with #isSatelliteType.
     */
    int isSatelliteTypeGlonass(unsigned long index) { return isSatelliteType(index, GLONASS); }

    /**
     * @brief Get satellite type
     * @details Specify the element number of the satellite and return the
     *          type of satellite system.
     * @param [in] index Array number of the satellite array
     * @return Type of satellite system
     */
    SpSatelliteType getSatelliteType(unsigned long index);

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
    signed short getSatelliteAzimuth(unsigned long index);

    /**
     * @brief Get satellite signal level(C/N)
     * @details Specify the element number of the satellite and return the SignalLevel value stored in the array.
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
     * @return 0 if success, -1 if failure
     */
    int begin(void);

    /**
     * @brief Activate GNSS device
     * @details Power on GNSS hardware block, and change to the state where parameter
     *          setting and positioning start can be performed.
     * @param [in] debugOut debug out stream instead of Serial
     * @return 0 if success, -1 if failure
     */
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
     * @brief Check position information is updated and return immediately
     * @return 1 enable, 0 disable
     *
     * Returns 1 if position information is updated.
     */
    int isUpdate(void);

    /**
     * @brief Wait for position information to be updated
     * @details Calling this function will block until GNSS positioning information is updated.
     * @param [in] timeout timeout of waiting [sec]
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
    unsigned long getPositionData(GnssPositionData *pData);
#ifdef CONFIG_CXD56_GNSS_ADDON
    unsigned long getPositionData(GnssPositionData2 *pData);
#endif

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
     * @param [in] interval Interval time[sec]
     * @return 0 if success, -1 if failure
     */
    int setInterval(long interval = 1);

    /**
     * @brief Set the pos interval time
     * @details Set interval of POS operation.
     * @param [in] interval Interval frequency
     * @return 0 if success, -1 if failure
     */
    int setInterval(SpIntervalFreq interval);

    /**
     * @brief Returns whether the specified satellite system is selecting
     * @details Returns whether satellite system specified as argument is
     *          selecting as the satellite used for positioning.
     * @param [in] sattype Type of satellite system
     * @return 1 selected, 0 not selected
     */
    int isSelecting(SpSatelliteType sattype);

    /**
     * @brief Add specified satellite system to selection for positioning
     * @details GPS is selected by default. In addition to this, it is able
     *          to select Glonass or QZSS L1 / CA as the positioning
     *          satellite system, SBAS or QZSS L1S as the positioning
     *          augmentation system.
     * @param [in] sattype Type of satellite system
     * @return 0 if success, negative if failure
     */
    int select(SpSatelliteType sattype);

    /**
     * @brief Remove specified satellite system to selection for positioning
     * @details Do not use the specified satellite system for positioning.
     * @param [in] sattype type of satellite system
     * @return 0 if success, negative if failure
     */
    int deselect(SpSatelliteType sattype);

    /**
     * @brief [Obsolete] Returns whether GPS is used as satellite system
     * @details This function is obsolete. Replace with #isSelecting.
     */
    int isGps(void) { return isSelecting(GPS); }

    /**
     * @brief [Obsolete] Use GPS for positioning
     * @details This function is obsolete. Replace with #select.
     */
    int useGps(void) { return select(GPS); }

    /**
     * @brief [Obsolete] Unuse GPS for positioning
     * @details This function is obsolete. Replace with #deselect.
     */
    int unuseGps(void) { return deselect(GPS); }

    /**
     * @brief [Obsolete] Returns whether Glonass is used as satellite system
     * @details This function is obsolete. Replace with #isSelecting.
     */
    int isGlonass(void) { return isSelecting(GLONASS); }

    /**
     * @brief [Obsolete] Use Glonass for positioning
     * @details This function is obsolete. Replace with #select.
     */
    int useGlonass(void) { return select(GLONASS); }

    /**
     * @brief [Obsolete] Unuse Glonass for positioning
     * @details This function is obsolete. Replace with #deselect.
     */
    int unuseGlonass(void) { return deselect(GLONASS); }

    /**
     * @brief Set debug mode
     * @details Print debug messages about GNSS controlling and positioning if not set 0 to argument.
     * @param [in] level debug mode
     * @return none
     */
    void setDebugMode(SpPrintLevel level);

    /**
     * @brief Save the data stored in the backup RAM to Flash
     * @return 0 if success, -1 if failure
     */
    int saveEphemeris(void);

    /**
     * @brief Remove the backup data stored in the Flash
     * @return 0 if success, -1 if failure
     */
    int removeEphemeris(void);

    /**
     * @brief Get the QZQSM DC report data
     * @return the pointer to DC Report structure if valid, otherwise NULL
     */
    void* getDCReport(void);

    /**
     * @brief Start 1PPS output
     * @return none
     */
    void start1PPS(void);

    /**
     * @brief Stop 1PPS output
     * @return none
     */
    void stop1PPS(void);

protected:
    int fd_;                          /* file descriptor */
    unsigned long SatelliteSystem;    /* satellite type */
    SpNavData NavData;                /* copy pos data */

    unsigned long inquireSatelliteType(void);

    /* Debug Print */
    static Stream& DebugOut;
    static SpPrintLevel DebugPrintLevel;
    inline static void printMessage(SpPrintLevel level, const char* str)
    {
        if (level <= DebugPrintLevel)
            DebugOut.print(str);
    }
};

#ifdef CONFIG_CXD56_GNSS_ADDON
class SpGnssAddon : public SpGnss {
public:
    int begin(void);
    int begin(Stream& debugOut) { DebugOut = debugOut; return begin(); }
};
#endif

/** @} gnss */

#endif // Gnss_h
