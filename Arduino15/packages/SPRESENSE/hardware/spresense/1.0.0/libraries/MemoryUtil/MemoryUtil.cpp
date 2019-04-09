/*
 *  MemoryUtil.cpp - SPI implement file for the Spresense SDK
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

#include "MemoryUtil.h"

#include "memutil/fixed_fence.h"
#include "memutil/msgq_pool.h"
#include "memutil/pool_layout.h"

#include <asmp/mpshm.h>

using namespace MemMgrLite;

static mpshm_t s_shm;

/*--------------------------------------------------------------------------*/
const PoolAttr *getPoolLayout(int layout_no)
{
  return MemoryPoolLayouts[layout_no];
}

/*--------------------------------------------------------------------------*/
int initMemoryPools(void)
{

  int ret = mpshm_init(&s_shm, 1,  RAM_TILE_SIZE * 2); /* Used 2 Tile */
  if (ret < 0)
    {
      printf("mpshm_init() failure. %d\n", ret);
      return 1;
    }

  uint32_t addr = SHM_SRAM_ADDR;

  ret = mpshm_remap(&s_shm, (void *)addr);
  if (ret < 0)
    {
      printf("mpshm_remap() failure. %d\n", ret);
      return 1;
    }

  /* Initalize MessageLib */

  MsgLib::initFirst(NUM_MSGQ_POOLS,MSGQ_TOP_DRM);

  MsgLib::initPerCpu();

  /* Initialize MemoryManager */

  void* mml_data_area = translatePoolAddrToVa(MEMMGR_DATA_AREA_ADDR);
  Manager::initFirst(mml_data_area, MEMMGR_DATA_AREA_SIZE);

  Manager::initPerCpu(mml_data_area, NUM_MEM_POOLS);

  return 0;
}

/*--------------------------------------------------------------------------*/
int createStaticPools(uint8_t layout_no)
{
  void* work_va = translatePoolAddrToVa(MEMMGR_WORK_AREA_ADDR);

  Manager::createStaticPools(layout_no,
                             work_va,
                             MEMMGR_MAX_WORK_SIZE,
                             getPoolLayout(layout_no));

  return 0;
}

/*--------------------------------------------------------------------------*/
int destroyStaticPools(void)
{
  Manager::destroyStaticPools();

  return 0;
}

/*--------------------------------------------------------------------------*/
int finalizeMemoryPools(void)
{
  /* Finalize MessageLib. */

  MsgLib::finalize();

  /* Destroy static pools. */

  MemMgrLite::Manager::destroyStaticPools();

  /* Finalize memory manager. */

  MemMgrLite::Manager::finalize();

  /* Destroy shared memory. */

  int ret;
  ret = mpshm_detach(&s_shm);
  if (ret < 0)
    {
      printf("Error: mpshm_detach() failure. %d\n", ret);
      return 1;
    }

  ret = mpshm_destroy(&s_shm);
  if (ret < 0)
    {
      printf("Error: mpshm_destroy() failure. %d\n", ret);
      return 1;
    }

  return 0;
}

