/*
  EEPROM.h - EEPROM library
  Copyright 2018 Sony Semiconductor Solutions Corporation
  Original Copyright (c) 2006 David A. Mellis.  All right reserved.
  New version by Christopher Andrews 2015.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef EEPROM_h
#define EEPROM_h

#ifdef SUBCORE
#error "EEPROM library is NOT supported by SubCore."
#endif

/**
 * @defgroup eeprom EEPROM Library API
 * @brief API for using EEPROM
 * @{
 */

#include <stdint.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <fcntl.h>

// Emulate EEPROM through a below file on SPI-Flash
#define EEPROM_EMU "/mnt/spif/eeprom.emu"

// Default size is the smaller than a sector of SPI-Flash.
// If you want the large capacity EEPROM, you can specify a larger size.
#define E2END 4000

/***
    EERef class.
    
    This object references an EEPROM cell.
    Its purpose is to mimic a typical byte of RAM, however its storage is the EEPROM.
    This class has an overhead of two bytes, similar to storing a pointer to an EEPROM cell.
***/

struct EERef{

    EERef( const int index )
        : index( index )                 {}
    
    //Access/read members.
    uint8_t operator*() const;
    void createInitialFile() const;
    operator uint8_t() const             { return **this; }
    
    //Assignment/write members.
    EERef &operator=( const EERef &ref ) { return *this = *ref; }
    EERef &operator=( uint8_t in );
    EERef &operator +=( uint8_t in )     { return *this = **this + in; }
    EERef &operator -=( uint8_t in )     { return *this = **this - in; }
    EERef &operator *=( uint8_t in )     { return *this = **this * in; }
    EERef &operator /=( uint8_t in )     { return *this = **this / in; }
    EERef &operator ^=( uint8_t in )     { return *this = **this ^ in; }
    EERef &operator %=( uint8_t in )     { return *this = **this % in; }
    EERef &operator &=( uint8_t in )     { return *this = **this & in; }
    EERef &operator |=( uint8_t in )     { return *this = **this | in; }
    EERef &operator <<=( uint8_t in )    { return *this = **this << in; }
    EERef &operator >>=( uint8_t in )    { return *this = **this >> in; }
    
    EERef &update( uint8_t in )          { return  in != *this ? *this = in : *this; }
    
    /** Prefix increment/decrement **/
    EERef& operator++()                  { return *this += 1; }
    EERef& operator--()                  { return *this -= 1; }
    
    /** Postfix increment/decrement **/
    uint8_t operator++ (int){ 
        uint8_t ret = **this;
        return ++(*this), ret;
    }

    uint8_t operator-- (int){ 
        uint8_t ret = **this;
        return --(*this), ret;
    }
    
    int index; //Index of current EEPROM cell.
};

/***
    EEPtr class.
    
    This object is a bidirectional pointer to EEPROM cells represented by EERef objects.
    Just like a normal pointer type, this can be dereferenced and repositioned using 
    increment/decrement operators.
***/

struct EEPtr{

    EEPtr( const int index )
        : index( index )                {}
        
    operator int() const                { return index; }
    EEPtr &operator=( int in )          { return index = in, *this; }
    
    //Iterator functionality.
    bool operator!=( const EEPtr &ptr ) { return index != ptr.index; }
    EERef operator*()                   { return index; }
    
    /** Prefix & Postfix increment/decrement **/
    EEPtr& operator++()                 { return ++index, *this; }
    EEPtr& operator--()                 { return --index, *this; }
    EEPtr operator++ (int)              { return index++; }
    EEPtr operator-- (int)              { return index--; }

    int index; //Index of current EEPROM cell.
};

/***
    EEPROMClass class.
    
    This object represents the entire EEPROM space.
    It wraps the functionality of EEPtr and EERef into a basic interface.
    This class is also 100% backwards compatible with earlier Arduino core releases.
***/

struct EEPROMClass{

    EEPROMClass() : initialized(0) {}
    //Create a eeprom emulation file if it doesn't exist
    void init() {
        int ret;
        FILE *fp = NULL;
        struct stat statBuf;
        long filesize = -1;
        long eepromsize = E2END;

        if (initialized != 0) {
            /* Already initialized */
            return;
        }

        /* Check whether the eeprom emulation file has already existed or not */
        if (0 == stat(EEPROM_EMU, &statBuf)) {
            filesize = statBuf.st_size;
        }

        if (eepromsize == filesize) {
            /* Already existed if the file size is equal to the eeprom size */
            initialized = 1;
            return;
        }

        /* Create a new file */
        if ((fp = fopen(EEPROM_EMU, "wb")) == NULL) {
            printf("ERROR: eeprom open failure\n");
        }

        uint8_t *ptr = (uint8_t*)zalloc(eepromsize);
        ret = fwrite(ptr, 1, eepromsize, fp);
        if (ret != eepromsize) {
            printf("ERROR: eeprom init failure (%d)\n", ret);
        }

        fclose(fp);

        initialized = 1;
        return;
    }
    //Remove the eeprom file and create the a zero-filled eeprom file
    void clear() {
        unlink(EEPROM_EMU);
        initialized = 0;
        init();
    }

    //Basic user access methods.
    EERef operator[]( const int idx )    { init(); return idx; }
    uint8_t read( int idx )              { init(); return EERef( idx ); }
    void write( int idx, uint8_t val )   { init(); (EERef( idx )) = val; }
    void update( int idx, uint8_t val )  { init(); EERef( idx ).update( val ); }

    //STL and C++11 iteration capability.
    EEPtr begin()                        { return 0x00; }
    EEPtr end()                          { return length(); } //Standards requires this to be the item after the last valid entry. The returned pointer is invalid.
    uint16_t length()                    { return E2END; }
    
    //Functionality to 'get' and 'put' objects to and from EEPROM.
    template< typename T > T &get( int idx, T &t ){
        int ret;
        FILE *fp = NULL;

        init();

        if ((fp = fopen(EEPROM_EMU, "rb")) == NULL) {
            printf("ERROR: eeprom open failure\n");
            goto errout;
        }

        ret = fseek(fp, idx, SEEK_SET);
        if (ret) {
            printf("ERROR: eeprom seek failure\n");
            goto errout_with_close;
        }

        ret = fread((uint8_t*)&t, 1, sizeof(T), fp);
        if (ret != sizeof(T)) {
            printf("ERROR: eeprom read failure (%d)\n", ret);
        }

    errout_with_close:
        fclose(fp);
    errout:
        return t;
    }

    template< typename T > const T &put( int idx, const T &t ){
        int ret;
        FILE *fp = NULL;

        init();

        if ((fp = fopen(EEPROM_EMU, "rb+")) == NULL) {
            printf("ERROR: eeprom open failure\n");
            goto errout;
        }

        ret = fseek(fp, idx, SEEK_SET);
        if (ret) {
            printf("ERROR: eeprom seek failure\n");
            goto errout_with_close;
        }

        ret = fwrite((uint8_t*)&t, 1, sizeof(T), fp);
        if (ret != sizeof(T)) {
            printf("ERROR: eeprom write failure (%d)\n", ret);
        }

    errout_with_close:
        fclose(fp);
    errout:
        return t;
    }

    int initialized;
};

extern EEPROMClass EEPROM;

/** @} eeprom */

#endif
