/* This file is generated automatically. */
/****************************************************************************
 * pool_layout.h
 *
 *   Copyright 2020 Sony Semiconductor Solutions Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Sony Semiconductor Solutions Corporation nor
 *    the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef POOL_LAYOUT_H_INCLUDED
#define POOL_LAYOUT_H_INCLUDED

#include "memutils/memory_manager/MemMgrTypes.h"

namespace MemMgrLite {

MemPool*  static_pools_block[NUM_MEM_SECTIONS][NUM_MEM_POOLS];
MemPool** static_pools[NUM_MEM_SECTIONS] = {
  static_pools_block[0],
  static_pools_block[1],
};
uint8_t layout_no[NUM_MEM_SECTIONS] = {
  BadLayoutNo,
  BadLayoutNo,
};
uint8_t pool_num[NUM_MEM_SECTIONS] = {
  NUM_MEM_S0_POOLS,
  NUM_MEM_S1_POOLS,
};
extern const PoolSectionAttr MemoryPoolLayouts[NUM_MEM_SECTIONS][NUM_MEM_LAYOUTS][17] = {
  {  /* Section:0 */
    {/* Layout:0 */
     /* pool_ID                          type         seg  fence  addr        size         */
      { S0_DEC_ES_MAIN_BUF_POOL        , BasicType  ,   4,  true, 0x000e0008, 0x00000fa0 },  /* COMMON_WORK_AREA */
      { S0_REND_PCM_BUF_POOL           , BasicType  ,   5,  true, 0x000e0fb0, 0x00010e00 },  /* COMMON_WORK_AREA */
      { S0_SRC_WORK_MAIN_BUF_POOL      , BasicType  ,   1,  true, 0x000f1db8, 0x00001200 },  /* COMMON_WORK_AREA */
      { S0_DEC_ES_SUB_BUF_POOL         , BasicType  ,   1,  true, 0x000f2fc0, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_REND_PCM_SUB_BUF_POOL       , BasicType  ,   1,  true, 0x000f2fd0, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_SRC_WORK_SUB_BUF_POOL       , BasicType  ,   1,  true, 0x000f2fe0, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_DEC_APU_CMD_POOL            , BasicType  ,  10,  true, 0x000f2ff0, 0x00000398 },  /* COMMON_WORK_AREA */
      { S0_PF0_PCM_BUF_POOL            , BasicType  ,   1,  true, 0x000f3390, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_PF1_PCM_BUF_POOL            , BasicType  ,   1,  true, 0x000f33a0, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_PF0_APU_CMD_POOL            , BasicType  ,  10,  true, 0x000f33b0, 0x00000398 },  /* COMMON_WORK_AREA */
      { S0_PF1_APU_CMD_POOL            , BasicType  ,  10,  true, 0x000f3750, 0x00000398 },  /* COMMON_WORK_AREA */
      { S0_NULL_POOL, 0, 0, false, 0, 0 },
    },
    {/* Layout:1 */
     /* pool_ID                          type         seg  fence  addr        size         */
      { S0_OUTPUT_BUF_POOL             , BasicType  ,   5,  true, 0x000e0008, 0x00003c00 },  /* COMMON_WORK_AREA */
      { S0_MIC_IN_BUF_POOL             , BasicType  ,   5,  true, 0x000e3c10, 0x00005a00 },  /* COMMON_WORK_AREA */
      { S0_ENC_APU_CMD_POOL            , BasicType  ,   3,  true, 0x000e9618, 0x00000114 },  /* COMMON_WORK_AREA */
      { S0_SRC_APU_CMD_POOL            , BasicType  ,   3,  true, 0x000e9738, 0x00000114 },  /* COMMON_WORK_AREA */
      { S0_PRE_APU_CMD_POOL            , BasicType  ,   3,  true, 0x000e9858, 0x00000114 },  /* COMMON_WORK_AREA */
      { S0_NULL_POOL, 0, 0, false, 0, 0 },
    },
    {/* Layout:2 */
     /* pool_ID                          type         seg  fence  addr        size         */
      { S0_DEC_ES_MAIN_BUF_POOL        , BasicType  ,   1,  true, 0x000e0008, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_REND_PCM_BUF_POOL           , BasicType  ,   1,  true, 0x000e0018, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_SRC_WORK_MAIN_BUF_POOL      , BasicType  ,   1,  true, 0x000e0028, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_DEC_ES_SUB_BUF_POOL         , BasicType  ,   1,  true, 0x000e0038, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_REND_PCM_SUB_BUF_POOL       , BasicType  ,   1,  true, 0x000e0048, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_SRC_WORK_SUB_BUF_POOL       , BasicType  ,   1,  true, 0x000e0058, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_DEC_APU_CMD_POOL            , BasicType  ,   1,  true, 0x000e0068, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_PF0_PCM_BUF_POOL            , BasicType  ,   1,  true, 0x000e0078, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_PF1_PCM_BUF_POOL            , BasicType  ,   1,  true, 0x000e0088, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_PF0_APU_CMD_POOL            , BasicType  ,   1,  true, 0x000e0098, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_PF1_APU_CMD_POOL            , BasicType  ,   1,  true, 0x000e00a8, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_OUTPUT_BUF_POOL             , BasicType  ,   1,  true, 0x000e00b8, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_MIC_IN_BUF_POOL             , BasicType  ,   1,  true, 0x000e00c8, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_ENC_APU_CMD_POOL            , BasicType  ,   1,  true, 0x000e00d8, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_SRC_APU_CMD_POOL            , BasicType  ,   1,  true, 0x000e00e8, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_PRE_APU_CMD_POOL            , BasicType  ,   1,  true, 0x000e00f8, 0x00000004 },  /* COMMON_WORK_AREA */
      { S0_NULL_POOL, 0, 0, false, 0, 0 },
    },
  },
  {  /* Section:1 */
    {/* Layout:0 */
     /* pool_ID                          type         seg  fence  addr        size         */
      { S1_SENSOR_DSP_CMD_BUF_POOL     , BasicType  ,   8, false, 0x000f3af0, 0x00001800 },  /* COMMON_WORK_AREA */
      { S1_SENSOR_DATA_BUF_POOL        , BasicType  ,   8, false, 0x000f52f0, 0x00001800 },  /* COMMON_WORK_AREA */
      { S1_NULL_POOL, 0, 0, false, 0, 0 },
    },
  },
}; /* end of MemoryPoolLayouts */

}  /* end of namespace MemMgrLite */

#endif /* POOL_LAYOUT_H_INCLUDED */
