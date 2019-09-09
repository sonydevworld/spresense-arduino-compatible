/*
 *  MemoryUtil.h - Audio include file for the Spresense SDK
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

#ifndef MemoryUtil_h
#define MemoryUtil_h

/* Define the following: Reduce memory usage to 1 tile (128 kbytes). */

//#define MEMORY_UTIL_TINY

#ifdef SUBCORE
#error "MemoryUtil library is NOT supported by SubCore."
#endif

#define _POSIX
#define USE_MEMMGR_FENCE

#include <memutils/memory_manager/MemHandle.h>
#include <memutils/message/Message.h>

#ifdef MEMORY_UTIL_TINY
#include "memutil/tiny/msgq_id.h"
#include "memutil/tiny/mem_layout.h"
#include "memutil/tiny/memory_layout.h"
#else
#include "memutil/msgq_id.h"
#include "memutil/mem_layout.h"
#include "memutil/memory_layout.h"
#endif

using namespace MemMgrLite;

/*--------------------------------------------------------------------------*/
/**
 * @class MemoryUtilClass
 * @brief MemoryManager Library Class Definitions.
 */

class MemoryUtilClass
{
public:

  MemoryUtilClass():m_state(E_Inactive){}
  
  /**
   * @brief Initialize MemoryManager library.
   *
   * @details Initialization of the entire MemoryManager. 
   *          Run only once on a single CPU.
   */

  int begin();


  /**
   * @brief Generate static memory pool group.
   *
   * @details Generate a memory pool group with the specified layout number.
   *
   */

  int setLayout(uint8_t sec_no, uint8_t layout_no);

  /**
   * @brief Destroy the static memory pool.
   *
   */

  int clearLayout();

  /**
   * @brief Finalize MemoryManager library.
   *
   */

  int end();

  /**
   * @brief Get static memory pool information.
   *
   * @details Get memory pool group with the specified layout number.
   *
   */

  const PoolSectionAttr* getLayout(uint8_t sec_no, int layout_no);

private:
  enum E_state{
    E_Inactive=0,
    E_Active,
    STATE_NUM
  };
  
  E_state m_state;
};

/*--------------------------------------------------------------------------*/
extern MemoryUtilClass MemoryUtil;

/*--------------------------------------------------------------------------*/
extern int initMemoryPools(void);
extern int createStaticPools(uint8_t layout_no);
extern int createStaticPools(uint8_t sec_no, uint8_t layout_no);
extern int destroyStaticPools(void);
extern int finalizeMemoryPools(void);
extern const PoolSectionAttr *getPoolLayout(int layout_no);
extern const PoolSectionAttr *getPoolLayout(uint8_t sec_no, int layout_no);


#endif /* MemoryUtil_h */

