/*
  ext_flash.c - write to external flash

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

#include <string.h>

#include "ext_flash.h"
#include "gpio1.h"

#define SS_PIN(inst) ((inst==0) ? M1_EZPCS : M2_SS)
#define SS_ENABLE(inst) (GPIO_DRV_ClearPinOutput(SS_PIN(inst)))
#define SS_DISABLE(inst) (GPIO_DRV_SetPinOutput(SS_PIN(inst)))
#define HAS_UNLOCK(inst) (inst == 1)
#define HAS_RESET(inst) (inst == 1)

void EXT_read_block(uint32_t instance, uint32_t address, uint8_t* buffer, uint32_t count)
{
    uint8_t txbuff[4];
    txbuff[0] = 0x03;//read address
    txbuff[1] = (address >> 16) & 0xFF;
    txbuff[2] = (address >> 8) & 0xFF;
    txbuff[3] = (address) & 0xFF;

    memset(buffer, 0xCC, count);

    SS_ENABLE(instance);
    SPI_DRV_MasterTransferBlocking(instance, NULL, txbuff, NULL, 4, 100);
    SPI_DRV_MasterTransferBlocking(instance, NULL, NULL, buffer, count, 100);
    SS_DISABLE(instance);
}

static void EXT_write_enable(uint32_t instance, bool enable)
{
    uint8_t txbuff[1] = {enable ? 0x06 : 0x04};
    SS_ENABLE(instance);
    SPI_DRV_MasterTransferBlocking(instance, NULL, txbuff, NULL, 1, 100);
    SS_DISABLE(instance);
}

static void EXT_poll_busy(uint32_t instance)
{
    uint8_t txbuff[2] = {0x05, 0x00};
    uint8_t rxbuff[2];
    //poll for write flag
    do{
        SS_ENABLE(instance);
        SPI_DRV_MasterTransferBlocking(instance, NULL, txbuff, rxbuff, 2, 100);
        SS_DISABLE(instance);
    }while(rxbuff[1] & 0x01);
}

void EXT_write_block(uint32_t instance, uint32_t address, uint8_t* buffer, uint32_t count)
{
    uint8_t txbuff[4];
    uint32_t offset = 0;

    while(count)
    {
        uint32_t towrite = count;
        if(towrite > 256)
            towrite = 256;
        count -= towrite;

        EXT_write_enable(instance, true);

        txbuff[0] = 0x02;//write address
        txbuff[1] = (address >> 16) & 0xFF;
        txbuff[2] = (address >> 8) & 0xFF;
        txbuff[3] = (address) & 0xFF;

        SS_ENABLE(instance);
        SPI_DRV_MasterTransferBlocking(instance, NULL, txbuff, NULL, 4, 100);
        SPI_DRV_MasterTransferBlocking(instance, NULL, buffer+offset, NULL, towrite, 2000);
        SS_DISABLE(instance);

        EXT_poll_busy(instance);

        EXT_write_enable(instance, false);

        address += towrite;
        offset += towrite;
    }

}

void EXT_erase_sector(uint32_t instance, uint32_t address)
{
    uint8_t txbuff[4];
    EXT_write_enable(instance, true);

    txbuff[0] = instance == 0 ? 0xD8 : 0x20;//erase sector
    txbuff[1] = (address >> 16) & 0xFF;
    txbuff[2] = (address >> 8) & 0xFF;
    txbuff[3] = (address) & 0xFF;

    SS_ENABLE(instance);
    SPI_DRV_MasterTransferBlocking(instance, NULL, txbuff, NULL, 4, 2000);
    SS_DISABLE(instance);
    EXT_poll_busy(instance);
    EXT_write_enable(instance, false);
}

void EXT_unlock(uint32_t instance)
{
    uint8_t txbuff[1] = {0x98};
    if(HAS_UNLOCK(instance))
    {
        EXT_write_enable(instance, true);
        SS_ENABLE(instance);
        SPI_DRV_MasterTransferBlocking(instance, NULL, txbuff, NULL, 1, 100);
        SS_DISABLE(instance);
        EXT_poll_busy(instance);
        EXT_write_enable(instance, false);
    }
}

void EXT_reset(uint32_t instance)
{
    uint8_t txbuff[1] = {0x66};
    if(HAS_RESET(instance))
    {
        SS_ENABLE(instance);
        SPI_DRV_MasterTransferBlocking(instance, NULL, txbuff, NULL, 1, 100);
        SS_DISABLE(instance);
        txbuff[0] = 0x99;
        SS_ENABLE(instance);
        SPI_DRV_MasterTransferBlocking(instance, NULL, txbuff, NULL, 1, 100);
        SS_DISABLE(instance);
    }
}

