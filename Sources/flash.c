/*
  flash.c - write to internal flash

  https://hologram.io

  Copyright (c) 2016 Konekt, Inc.  All rights reserved.

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

#include "flash.h"

#include <string.h>

#include "flash1.h"

#define LAUNCH_CMD_SIZE           0x100
#define ONE_KB                    1024
#define P_FLASH_SIZE            (FSL_FEATURE_FLASH_PFLASH_BLOCK_SIZE * FSL_FEATURE_FLASH_PFLASH_BLOCK_COUNT)
pFLASHCOMMANDSEQUENCE g_FlashLaunchCommand = (pFLASHCOMMANDSEQUENCE)0xFFFFFFFF;
uint16_t ramFunc[LAUNCH_CMD_SIZE/2];

void FLASH_init_ram(void)
{
    g_FlashLaunchCommand = (pFLASHCOMMANDSEQUENCE)RelocateFunction((uint32_t)ramFunc, LAUNCH_CMD_SIZE, (uint32_t)FlashCommandSequence);
}

bool FLASH_erase_sector(uint32_t sector_address)
{
    uint32_t result;

    __disable_irq();
    result = FlashEraseSector(&flash1_InitConfig0, sector_address, FTFx_PSECTOR_SIZE, g_FlashLaunchCommand);
    __enable_irq();

//    if(result != 0)
//        printf("ERASE FAIL: %d\r\n", result);

    return result == 0;
}

bool FLASH_write_block(uint32_t address, uint8_t *block, uint32_t size)
{
    uint32_t result;

    __disable_irq();
    result = FlashProgram(&flash1_InitConfig0, address, size, block,
            g_FlashLaunchCommand);
    __enable_irq();
    if(result == 0)
        return memcmp(block, (void*)address, size) == 0;
    return false;
}
