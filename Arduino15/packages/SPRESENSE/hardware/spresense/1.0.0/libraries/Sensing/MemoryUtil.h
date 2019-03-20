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

#ifndef MemoryUtis_h
#define MemoryUtis_h

// #ifdef __cplusplus

#define _POSIX
#define USE_MEMMGR_FENCE

#include <memutils/memory_manager/MemHandle.h>
#include <memutils/message/Message.h>

#include "memutil/msgq_id.h"
#include "memutil/mem_layout.h"

#define RAM_TILE_SIZE   (1024*128)

using namespace MemMgrLite;

/*--------------------------------------------------------------------------*/
const PoolAttr *getPoolLayout(int layout_no);
int initMemoryPools(void);
int createStaticPools(uint8_t layout_no);
int destroyStaticPools(void);

// #endif /* __cplusplus */
#endif /* MemoryUtis_h */

