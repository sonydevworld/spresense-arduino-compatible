/*
 *  GNSS.cpp - GNSS implementation file for the Spresense SDK
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

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arch/chip/gnss.h>
#include <GNSS.h>
#include <GNSSPositionData.h>
#include <Arduino.h>
#include <signal.h>
#include <poll.h>
#include <arch/board/board.h>

#define SP_GNSS_DEBUG

#ifdef SP_GNSS_DEBUG /* switch debug message on/off */
# define PRINT_E(s) SpGnss::printMessage(PrintError, s);
# define PRINT_W(s) SpGnss::printMessage(PrintWarning, s);
# define PRINT_I(s) SpGnss::printMessage(PrintInfo, s);
#else
# define PRINT_E(c)
# define PRINT_W(c)
# define PRINT_I(c)
#endif /* SP_GNSS_DEBUG */

//#define SP_GNSS_USE_SIGNAL
#define GNSS_POLL_FD_NUM 1

const char SP_GNSS_DEV_NAME[]       = "/dev/gps";
const char SP_GNSS_DEV2_NAME[]      = "/dev/gps2";
const int SP_GNSS_SIG               = 18;
const unsigned int MAGIC_NUMBER     = 0xDEADBEEF;
const unsigned int BIN_BUF_SIZE     = sizeof(GnssPositionData);
#ifdef CONFIG_CXD56_GNSS_ADDON
const unsigned int BIN_BUF_SIZE2    = sizeof(GnssPositionData2);
#endif

SpPrintLevel SpGnss::DebugPrintLevel = PrintNone;   /* Print level */
Stream& SpGnss::DebugOut = Serial;

#ifdef SP_GNSS_USE_SIGNAL
static struct cxd56_gnss_signal_setting_s setting;
static sigset_t mask;
static int no_handler = 0;
#endif
static struct cxd56_gnss_positiondata_s *pPosdat = NULL;
static uint32_t crc_table[256];

/*
 * Init CRC
 */
static void make_crc_table(void) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++) {
            c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
        }
        crc_table[i] = c;
    }
}

/*
 * Calculate CRC
 */
static uint32_t crc32(uint8_t *buf, size_t len) {
    uint32_t c = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        c = crc_table[(c ^ buf[i]) & 0xFF] ^ (c >> 8);
    }
    return c ^ 0xFFFFFFFF;
}

#if defined(_ENABLE_TIME_T)
/**
 * @brief
 * @return
 */
static struct cxd56_gnss_datetime_s ConvetTime(time_t sec)
{
    struct tm *gettime;
    struct cxd56_gnss_datetime_s rettime;

    /* convert time */
    gettime = localtime(&sec);

    rettime.date.year   = gettime->tm_year;
    rettime.date.month  = gettime->tm_mon;
    rettime.date.day    = gettime->tm_hour;
    rettime.time.hour   = gettime->tm_mday;
    rettime.time.minute = gettime->tm_min;
    rettime.time.sec    = gettime->tm_sec;
    rettime.time.usec   = 0;

    return rettime;
}
#endif /* if defined(_ENABLE_TIME_T) */

/**
 * @brief Returns whether specified type of satellite is used
 * @return 1 use, 0 unuse
 */
int SpNavData::isSatelliteType(unsigned long index, SpSatelliteType sattype)
{
    int ret;

    if (satellite[index].type & sattype)
    {
        ret = true;
    }
    else
    {
        ret = false;
    }

    return ret;
}

/**
 * @brief Get satellite type
 * @param [in] index Array number of the satellite array
 * @return Type of satellite system
 */
SpSatelliteType SpNavData::getSatelliteType(unsigned long index)
{
    if(index < numSatellites)
    {
        return (SpSatelliteType)satellite[index].type;
    }
    else
    {
        PRINT_E("SpNavData E: invalid range!!\n");
        return UNKNOWN;
    }
}

/**
 * @brief Specify the element number of the satellite and return the SatelliteId value stored in the array
 * @param [in] index Array index of the satellite array
 * @return Value of array SatelliteId
 */
unsigned char SpNavData::getSatelliteId(unsigned long index)
{
    if(index < numSatellites)
    {
        return satellite[index].svid;
    }
    else
    {
        PRINT_E("SpNavData E: invalid range!!\n");
        return 0;
    }
}

/**
 * @brief Specify the element number of the satellite and return the SatelliteElevation value stored in the array
 * @param [in] index Array index of the satellite array
 * @return Value of array SatelliteElevation
 */
unsigned char SpNavData::getSatelliteElevation(unsigned long index)
{
    if(index < numSatellites)
    {
        return satellite[index].elevation;
    }
    else
    {
        PRINT_E("SpNavData E: invalid range!!\n");
        return 0;
    }
}

/**
 * @brief Specify the element number of the satellite and return the SatelliteAzimuth value stored in the array
 * @param [in] index Array index of the satellite array
 * @return Value of array SatelliteAzimuth
 */
signed short SpNavData::getSatelliteAzimuth(unsigned long index)
{
    if(index < numSatellites)
    {
        return satellite[index].azimuth;
    }
    else
    {
        PRINT_E("SpNavData E: invalid range!!\n");
        return 0;
    }
}

/**
 * @brief Specify the element number of the satellite and return the SignalLevel value stored in the array
 * @param [in] index Array index of the satellite array
 * @return Value of array SatelliteSignalLevel
 */
float SpNavData::getSatelliteSignalLevel(unsigned long index)
{
    if(index < numSatellites)
    {
        return satellite[index].sigLevel;
    }
    else
    {
        PRINT_E("SpNavData E: invalid range!!\n");
        return 0.0;
    }
}

SpGnss::SpGnss()
  : fd_(-1),                            /* invalid value */
    SatelliteSystem(CXD56_GNSS_SAT_GPS) /* enable GPS */
{
    DebugPrintLevel = PrintError;       /* debug mode */
}

SpGnss::~SpGnss()
{
    (void) end();
}

#ifdef SP_GNSS_USE_SIGNAL
static void signal_handler( int no )
{
    no_handler = no;
}
#endif

/**
 * @brief Power on GNSS hardware block
 * @return 0 if success, -1 if failure
 */
int SpGnss::begin(void)
{
    PRINT_I("SpGnss : begin in\n");

    if (fd_ < 0)
    {
        fd_ = open(SP_GNSS_DEV_NAME, O_RDONLY);
        if (fd_ < 0)
        {
            PRINT_E("SpGnss E: Failed to open gps device\n");
            return -1;
        }
    }
#ifdef SP_GNSS_USE_SIGNAL
    int ret;

    /* Configure mask to notify GNSS signal. */

    sigemptyset(&mask);
    sigaddset(&mask, SP_GNSS_SIG);
    if ((ret = sigprocmask(SIG_UNBLOCK, &mask, NULL)) < 0)
    {
        PRINT_E("sigprocmask failed.\n");
        end();
        return ret;
    }
    else
    {
        /* Set the signal to notify GNSS events. */

        setting.fd      = fd_;
        setting.enable  = 1;
        setting.gnsssig = CXD56_GNSS_SIG_GNSS;
        setting.signo   = SP_GNSS_SIG;
        setting.data    = NULL;

        ret = ioctl(fd_, CXD56_GNSS_IOCTL_SIGNAL_SET, (unsigned long)&setting);
        if(ret < 0)
        {
            PRINT_E("SpGnss E: SIGNAL_SET error\n");
            end();
            return ret;
        }

        struct sigaction sa;
        sa.sa_handler = signal_handler;
        sa.sa_flags = SA_NOCLDSTOP;
        sa.sa_mask = mask;
        sigaction( SP_GNSS_SIG,  &sa, 0 );
    }
#endif
    /* Init CRC. */

    make_crc_table();

    /* Alloc buffer. */

    if(pPosdat == NULL)
    {
        pPosdat = (struct cxd56_gnss_positiondata_s*)malloc(sizeof(struct cxd56_gnss_positiondata_s));
        if(pPosdat == NULL)
        {
            PRINT_E("SpGnss E: Alloc error\n");
            end();
            return -1;
        }
    }

    PRINT_I("SpGnss : begin out\n");
    return OK;
}

/**
 * @brief Power off GNSS hardware block
 * @return 0 if success, -1 if failure
 */
int SpGnss::end(void)
{
    PRINT_I("SpGnss : end in\n");
    /* Check argument. */

    if (fd_ < 0)
    {
        PRINT_E("SpGnss E: not initialized!\n");
        return -1;
    }

    int ret;

    ret = close(fd_);
    if (ret < OK)
    {
        PRINT_E("SpGnss E: Failed to close gps device\n");
    }
    else
    {
        fd_ = -1;
    }

    /* Free buffer. */

    if(pPosdat != NULL)
    {
        free((void*)pPosdat);
        pPosdat = NULL;
    }

    PRINT_I("SpGnss : end out\n");
    return ret;
}

/**
 * @brief Start positioning
 * @return 0 if success, -1 if failure
 *
 * If not specified mode, force set to hot start mode.
 */
int SpGnss::start(SpStartMode mode)
{
    PRINT_I("SpGnss : start in\n");
    /* Check argument. */

    if (fd_ < 0)
    {
        PRINT_E("SpGnss E: not initialized!\n");
        return -1;
    }

    if (SatelliteSystem == 0)
    {
        PRINT_E("SpGnss E: no satellite mode!\n");
        return -1;
    }

    int ret;
    long startmode;

    /* Call ioctl. */

    ret = ioctl(fd_, CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM, SatelliteSystem);
    if (ret < 0)
    {
        PRINT_E("SpGnss E: Failed to set satellite\n");
        return ret;
    }

    /* Convert parameter. */

    switch(mode)
    {
    case COLD_START:
        startmode = CXD56_GNSS_STMOD_COLD;
        PRINT_I("  mode = COLD_START\n");
        break;
    case WARM_START:
        startmode = CXD56_GNSS_STMOD_WARM;
        PRINT_I("  mode = WARM_START\n");
        break;
    case HOT_START:
        startmode = CXD56_GNSS_STMOD_HOT;
        PRINT_I("  mode = HOT_START\n");
        break;
    default:
        startmode = -1;
        break;
    }

    if (startmode == -1)
    {
        PRINT_E("SpGnss E: Invalid argument!\n");
        return -1;
    }

    /* Call ioctl. */

    ret = ioctl(fd_, CXD56_GNSS_IOCTL_START, startmode);
    if (ret < OK)
    {
        PRINT_E("SpGnss E: Failed to start GNSS\n");
    }

    PRINT_I("SpGnss : start out\n");
    return ret;
}

/**
 * @brief Stop positioning
 * @return 0 if success, -1 if failure
 */
int SpGnss::stop(void)
{
    PRINT_I("SpGnss : stop in\n");
    /* Check argument. */

    if (fd_ < 0)
    {
        PRINT_E("SpGnss E: not initialized!\n");
        return -1;
    }

    int ret;

    /* Call ioctl. */

    ret = ioctl(fd_, CXD56_GNSS_IOCTL_STOP, 0);
    if (ret < OK)
    {
        PRINT_E("SpGnss E: Failed to stop GNSS\n");
    }

    PRINT_I("SpGnss : stop out\n");
    return ret;
}

/**
 * @brief Check position information is updated and return immediately
 * @return 1 enable, 0 disable
 *
 * Returns 1 if position information is updated.
 */
int SpGnss::isUpdate(void)
{
    return waitUpdate(0);
}

/**
 * @brief Wait for position information to be updated
 * @param [in] timeout timeout of waiting [sec]
 * @return 1 enable, 0 disable
 *
 * If not specified timeout, wait forever.
 */
int SpGnss::waitUpdate(long timeout)
{
    int ret = 0;
#ifdef SP_GNSS_USE_SIGNAL
    int sig_ret = 0;

    /* Check update */

    if(no_handler == SP_GNSS_SIG){
        sig_ret = SP_GNSS_SIG;
    }
    else if(timeout < 0)
    {
        sig_ret = sigtimedwait(&mask, NULL, NULL);
    }
    else if(timeout > 0)
    {
        struct timespec time;
        time.tv_sec  = timeout;
        time.tv_nsec = 0;
        sig_ret = sigtimedwait(&mask, NULL, &time);
    }

    if (sig_ret == SP_GNSS_SIG){
        ret = 1;
        no_handler = 0;
    }
    return ret;
#else
    struct pollfd fds[GNSS_POLL_FD_NUM];
    int msec;

    msec = (timeout > 0) ? (int)(timeout * 1000) : timeout;

    fds[0].fd     = fd_;
    fds[0].events = POLLIN;

    ret = poll(fds, GNSS_POLL_FD_NUM, msec);

    return (ret > 0) ? 1 : 0;
#endif
}

/**
 * @brief Get updated Nav data from GNSS
 * @param [out] NavData includes latest positioning data
 * @return none
 *
 * Give NavData the address of the object instantiated in your app as an argument.
 * The latest position information when the function is called is stored into this object.
 */
void SpGnss::getNavData(SpNavData *pNavData)
{
    int ret;
    unsigned long cnt;

    if(pPosdat == NULL)
    {
        PRINT_E("SpGnss E: Invalid argument\n");
        return;
    }

    /* Read pos data. */

    ret = read(fd_, pPosdat, sizeof(struct cxd56_gnss_positiondata_s));
    if (ret < 0)
    {
        PRINT_E("SpGnss E: Failed to read position data\n");
    }
    else
    {
        /* Copy to Navidata. */
        NavData.time.year   = pPosdat->receiver.date.year;
        NavData.time.month  = pPosdat->receiver.date.month;
        NavData.time.day    = pPosdat->receiver.date.day;
        NavData.time.hour   = pPosdat->receiver.time.hour;
        NavData.time.minute = pPosdat->receiver.time.minute;
        NavData.time.sec    = pPosdat->receiver.time.sec;
        NavData.time.usec   = pPosdat->receiver.time.usec;

        NavData.type           = pPosdat->receiver.type;
        NavData.posFixMode     = pPosdat->receiver.pos_fixmode;
        NavData.posDataExist   = pPosdat->receiver.pos_dataexist;
        NavData.numSatellitesCalcPos    = pPosdat->receiver.numsv_calcpos;
        NavData.satelliteType  = pPosdat->receiver.svtype;
        NavData.posSatelliteType        = pPosdat->receiver.pos_svtype;
        NavData.latitude       = pPosdat->receiver.latitude;
        NavData.longitude      = pPosdat->receiver.longitude;
        NavData.altitude       = pPosdat->receiver.altitude;
        NavData.velocity       = pPosdat->receiver.velocity;
        NavData.direction      = pPosdat->receiver.direction;

        NavData.pdop           = pPosdat->receiver.pos_dop.pdop;
        NavData.hdop           = pPosdat->receiver.pos_dop.hdop;
        NavData.vdop           = pPosdat->receiver.pos_dop.vdop;
        NavData.tdop           = pPosdat->receiver.pos_dop.tdop;

        NavData.numSatellites  = min(pPosdat->svcount, 24);
        for (cnt=0; cnt<NavData.numSatellites; cnt++)
        {
            NavData.satellite[cnt].type        = pPosdat->sv[cnt].type;
            NavData.satellite[cnt].svid        = pPosdat->sv[cnt].svid;
            NavData.satellite[cnt].elevation   = pPosdat->sv[cnt].elevation;
            NavData.satellite[cnt].azimuth     = pPosdat->sv[cnt].azimuth;
            NavData.satellite[cnt].sigLevel    = pPosdat->sv[cnt].siglevel;
        }
        for (; cnt<24; cnt++)
        {
            NavData.satellite[cnt].type        = 0;
            NavData.satellite[cnt].svid        = 0;
            NavData.satellite[cnt].elevation   = 0;
            NavData.satellite[cnt].azimuth     = 0;
            NavData.satellite[cnt].sigLevel    = 0;
        }
    }

    *pNavData = NavData;

    return;
}

/*
 * Get position data size.
 */
unsigned long SpGnss::getPositionDataSize(void)
{
    return BIN_BUF_SIZE;
}

/*
 * Get position data.
 */
unsigned long SpGnss::getPositionData(char *pBinaryBuffer)
{
    int ret;
    GnssPositionData *pPositionDataBuffer = (GnssPositionData*)pBinaryBuffer;

    /* Set magic number */

    pPositionDataBuffer->MagicNumber = MAGIC_NUMBER;

    /* Read pos data. */

    ret = read(fd_, &pPositionDataBuffer->Data, sizeof(struct cxd56_gnss_positiondata_s));
    if (ret <= 0)
    {
        PRINT_E("SpGnss E: Failed to read position data\n");
        return 0;
    }

    /* Set CRC */

    pPositionDataBuffer->CRC = crc32((uint8_t*)&pPositionDataBuffer->Data, sizeof(struct cxd56_gnss_positiondata_s));

    return BIN_BUF_SIZE;
}

unsigned long SpGnss::getPositionData(GnssPositionData *pPositionDataBuffer)
{
    return getPositionData((char *)pPositionDataBuffer);
}

#ifdef CONFIG_CXD56_GNSS_ADDON
unsigned long SpGnss::getPositionData(GnssPositionData2 *pPositionDataBuffer)
{
    int ret;

    /* Set magic number */

    pPositionDataBuffer->MagicNumber = MAGIC_NUMBER;

    /* Read pos data. */

    ret = read(fd_, &pPositionDataBuffer->Data, sizeof(pPositionDataBuffer->Data));
    if (ret <= 0)
    {
        PRINT_E("SpGnss E: Failed to read position data\n");
        return 0;
    }

    /* Set CRC */

    pPositionDataBuffer->CRC = crc32((uint8_t*)&pPositionDataBuffer->Data, sizeof(pPositionDataBuffer->Data));

    return BIN_BUF_SIZE2;
}
#endif

/**
 * @brief Set the current position for hot start
 * @param [in] latitude Latitude of current position
 * @param [in] longitude Longitude of current position
 * @param [in] altitude Altitude of current position
 * @return 0 if success, -1 if failure
 */
int SpGnss::setPosition(double latitude, double longitude, double altitude)
{
    int ret;
    struct cxd56_gnss_ellipsoidal_position_s position;

    /* Set parameter. */

    position.latitude  = latitude;
    position.longitude = longitude;
    position.altitude  = altitude;

    /* Call ioctl. */

    ret = ioctl(fd_, CXD56_GNSS_IOCTL_SET_RECEIVER_POSITION_ELLIPSOIDAL, (unsigned long)&position);
    if (ret < OK)
    {
        PRINT_E("SpGnss E: Failed to set Time\n");
    }

    return ret;
}

/**
 * @brief Set the current time for hot start
 * @param [in] time current time(SpGnssTime)
 * @return 0 if success, -1 if failure
 */
int SpGnss::setTime(SpGnssTime *time)
{
    int ret;
    struct cxd56_gnss_datetime_s settime;

    /* Set parameter. */

    settime.date.year   = time->year;
    settime.date.month  = time->month;
    settime.date.day    = time->day;
    settime.time.hour   = time->hour;
    settime.time.minute = time->minute;
    settime.time.sec    = time->sec;
    settime.time.usec   = time->usec;

    /* Call ioctl. */

    ret = ioctl(fd_, CXD56_GNSS_IOCTL_SET_TIME, (unsigned long)&settime);
    if (ret < OK)
    {
        PRINT_E("SpGnss E: Failed to set Time\n");
    }

    return ret;
}

#if defined(_ENABLE_TIME_T)
/**
 * @brief Set the current time for hot start
 * @param [in] time current time(time_t)
 * @return 0 if success, -1 if failure
 */
int SpGnss::setTime(time_t sec)
{
    int ret;
    struct cxd56_gnss_datetime_s settime;

    /* Set parameter. */

    settime = ConvetTime(sec);

    /* Call ioctl. */

    ret = ioctl(fd_, CXD56_GNSS_IOCTL_SET_TIME, (unsigned long)&settime);
    if (ret < OK)
    {
        PRINT_E("SpGnss E: Failed to set Time\n");
    }

    return ret;
}
#endif /* if defined(_ENABLE_TIME_T) */

/**
 * @brief Set the pos interval time
 * @details Set interval of POS operation.
 * @param [in] Interval time[sec]
 * @return 0 if success, -1 if failure
 */
int SpGnss::setInterval(long interval)
{
    int ret;
    struct cxd56_gnss_ope_mode_param_s setdata;

    /* Set parameter. */

      setdata.mode     = 1;
      setdata.cycle    = interval * 1000;

    /* Call ioctl. */

    ret = ioctl(fd_, CXD56_GNSS_IOCTL_SET_OPE_MODE, (unsigned long)&setdata);
    if (ret < OK)
    {
        PRINT_E("SpGnss E: Failed to set Interval\n");
    }

    return ret;
}

/**
 * @brief Set the pos interval time
 * @details Set interval of POS operation.
 * @param [in] Interval time[sec]
 * @return 0 if success, -1 if failure
 */
int SpGnss::setInterval(SpIntervalFreq interval)
{
    int ret;
    struct cxd56_gnss_ope_mode_param_s setdata;

    /* Set parameter. */

      setdata.mode     = 1;
      setdata.cycle    = (uint32_t)interval;

    /* Call ioctl. */

    ret = ioctl(fd_, CXD56_GNSS_IOCTL_SET_OPE_MODE, (unsigned long)&setdata);
    if (ret < OK)
    {
        PRINT_E("SpGnss E: Failed to set Interval\n");
    }

    return ret;
}

/**
 * @brief Returns whether the specified satellite system is selecting
 * @return 1 use, 0 unuse
 */
int SpGnss::isSelecting(SpSatelliteType sattype)
{
    int ret;

    if (SatelliteSystem & sattype)
    {
        ret = true;
    }
    else
    {
        ret = false;
    }

    return ret;
}

/**
 * @brief Add specified satellite system to selection for positioning
 * @return 0 if success, -1 if failure
 */
int SpGnss::select(SpSatelliteType sattype)
{
    unsigned long selection = SatelliteSystem | sattype;

    /* Call ioctl. */

    if (fd_ < 0)
    {
        PRINT_E("SpGnss E: not initialized!\n");
        return -1;
    }

    int ret = ioctl(fd_, CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM, selection);
    if (ret < 0)
    {
        PRINT_E("SpGnss E: Failed to set satellite\n");
        return ret;
    }

    SatelliteSystem = selection;

    return OK;
}

/**
 * @brief Remove specified satellite system to selection for positioning
 * @return 0 if success, -1 if failure
 */
int SpGnss::deselect(SpSatelliteType sattype)
{

    unsigned long selection = SatelliteSystem & ~sattype;

    if (selection == 0)
    {
        PRINT_W("SpGnss W: No satellite system.Please set any satellite.\n");
        return -1;
    }

    /* Call ioctl. */

    if (fd_ < 0)
    {
        PRINT_E("SpGnss E: not initialized!\n");
        return -1;
    }

    int ret = ioctl(fd_, CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM, selection);
    if (ret < 0)
    {
        PRINT_E("SpGnss E: Failed to set satellite\n");
        return ret;
    }

    SatelliteSystem = selection;

    return OK;
}

/**
 * @brief Set debug mode
 *        Print debug messages about GNSS controlling and positioning if not set 0 to argument.
 * @param [in] level debug print mode
 * @return none
 */
void SpGnss::setDebugMode(SpPrintLevel level)
{
    DebugPrintLevel = level;

    return ;
}

/**
 * @brief Save the data stored in the backup RAM to Flash
 * @return 0 if success, -1 if failure
 */
int SpGnss::saveEphemeris(void)
{
    int ret;

    /* Call ioctl. */

    ret = ioctl(fd_, CXD56_GNSS_IOCTL_SAVE_BACKUP_DATA, 0);
    if (ret < OK)
    {
        PRINT_E("SpGnss E: Failed to save BackupData\n");
    }

    return ret;
}

/**
 * @brief Remove the backup data stored in the Flash
 * @return 0 if success, -1 if failure
 */
int SpGnss::removeEphemeris(void)
{
#ifdef CONFIG_CXD56_GNSS_BACKUP_FILENAME
    return unlink(CONFIG_CXD56_GNSS_BACKUP_FILENAME);
#else
    return -1;
#endif
}

/**
 * @brief Get the QZQSM DC report data
 * @return the pointer to DC Report structure if valid, otherwise NULL
 */
void* SpGnss::getDCReport(void)
{
    int ret;
    static struct cxd56_gnss_dcreport_data_s s_dcreport;
    struct cxd56_gnss_dcreport_data_s dcreport;

    ret = lseek(fd_, CXD56_GNSS_READ_OFFSET_DCREPORT, SEEK_SET);
    if (ret < 0)
    {
        return NULL;
    }

    ret = read(fd_, &dcreport, sizeof(dcreport));
    if (ret < 0)
    {
        return NULL;
    }

    if (dcreport.svid == 0)
    {
        /* invalid data */
        return NULL;
    }

    if (0 == memcmp(&s_dcreport, &dcreport, sizeof(dcreport)))
    {
        /* not updated */
        return NULL;
    }

    memcpy(&s_dcreport, &dcreport, sizeof(dcreport));
    return &s_dcreport;
}

/**
 * @brief Start 1PPS output
 * @return none
 */
void SpGnss::start1PPS(void)
{
    int ret;

    ret = ioctl(fd_, CXD56_GNSS_IOCTL_SET_1PPS_OUTPUT, 1);
    if (ret < 0)
    {
        PRINT_E("SpGnss E: 1PPS start error\n");
    }
}

/**
 * @brief Stop 1PPS output
 * @return none
 */
void SpGnss::stop1PPS(void)
{
    int ret;

    ret = ioctl(fd_, CXD56_GNSS_IOCTL_SET_1PPS_OUTPUT, 0);
    if (ret < 0)
    {
        PRINT_E("SpGnss E: 1PPS stop error\n");
    }
}

/*
 * Private function: inquire
 */

unsigned long SpGnss::inquireSatelliteType(void)
{
    if (fd_ < 0)
    {
        return 0;
    }
    unsigned long sattype;
    int ret = ioctl(fd_, CXD56_GNSS_IOCTL_GET_SATELLITE_SYSTEM, (unsigned long)&sattype);
    if (ret < OK)
    {
        PRINT_E("SpGnss E: Failed to save BackupData\n");
    }

    return sattype;
}

#ifdef CONFIG_CXD56_GNSS_ADDON
int SpGnssAddon::begin(void)
{
    board_gnss_addon_initialize(SP_GNSS_DEV2_NAME, 0);
    fd_ = open(SP_GNSS_DEV2_NAME, O_RDONLY);
    if (fd_ < 0)
    {
        PRINT_E("SpGnssAddon E: Failed to open gps device\n");
        return -1;
    }
    return SpGnss::begin();
}
#endif
