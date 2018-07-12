/*
 *  gnss_file.h - Handling I/O operation on the SD card
 *  Copyright 2017 Sony Semiconductor Solutions Corporation
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

#ifndef _GNSS_FILE_H
#define _GNSS_FILE_H

/**
 * @file gnss_file.h
 * @author Sony Corporation
 * @brief Handling I/O operation on the SD card
 */

/* include the SDHCI library */
#include <SDHCI.h>

/**
 * @brief Mount SD card.
 * 
 * @return true if success, false if failure
 */
boolean BeginSDCard(void);

/**
 * @brief Write binary data to SD card.
 * 
 * @param [in] pBuff %Buffer to be written
 * @param [in] pName File name
 * @param [in] write_size Bytes to be written
 * @param [in] flag File access mode
 * @return Bytes written
 */
int WriteBinary(const char* pBuff, const char* pName, unsigned long write_size, int flag);

/**
 * @brief Write character string data to SD card.
 * 
 * @param [in] pBuff Character string to be written
 * @param [in] pName File name
 * @param [in] flag File access mode
 * @return Bytes written
 */
int WriteChar(const char* pBuff, const char* pName, int flag);

/**
 * @brief Read character string data from SD card.
 * 
 * @param [out] pBuff %Buffer where the read content will be stored.
 * @param [in] BufferSize Amount of bytes to read
 * @param [in] pName File name
 * @param [in] flag File access mode
 * @return The total amount of bytes read. -1 if failure.
 */
int ReadChar(char* pBuff, int BufferSize, const char* pName, int flag);

/**
 * @brief Remove file.
 * 
 * @param [in] pName File name
 * @return true if success, false if failure
 */
int Remove(const char* pName);

/**
 * @brief Check file exist from SD card.
 * 
 * @param [in] pName File name
 * @return true if exist, false if not
 */
boolean IsFileExist(const char* pName);

#endif

