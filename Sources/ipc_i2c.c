/*
  ipc_i2c.c - I2C Slave callback handlers

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
#include "ipc_i2c.h"

#include <string.h>
#include "i2cCom1.h"
#include "flash.h"
#include "gpio1.h"
#include "jump.h"
#include "spiComEZPort.h"
#include "ext_flash.h"
#include "boot.h"

#define STI2C_IDLE  0   // waiting
#define STI2C_CMD   1   // receiving command
#define STI2C_RX    2   // receiving data
#define STI2C_TX    3   // sending data

#define CMDI2C_NONE                     0x00
#define CMDI2C_READ_STATUS              0x01
#define CMDI2C_WRITE_SYSTEM_BLOCK       0x02
#define CMDI2C_USER_NOTIFY              0x22
#define CMDI2C_RESET                    0x55
#define CMDI2C_SYSTEMBOOT_VERSION       0x42
#define CMDI2C_SYSTEMFIRMWARE_VERSION   0x43

#define FLAG_WRITE_SYSTEM           0x0001
#define FLAG_RESET                  0x0004
#define FLAG_USER_NOTIFY            0x0008


typedef struct
{
    uint8_t command;    // Last received command
    uint8_t state;      // State of I2C handler
} i2c_callback_data_t;

typedef struct
{
    uint8_t busy :1;
    uint8_t error :1;
    uint8_t :6;
} i2c_status_reg_t;

typedef union
{
    i2c_status_reg_t fields;
    uint8_t byte;
} i2c_status_reg_u;

typedef struct
{
    uint8_t block_hi;
    uint8_t block_low;
    uint8_t block[1024];
} i2c_block_t;

//typedef struct
//{
//    uint32_t size;
//    char filename[48];
//}i2c_image_t;

static i2c_callback_data_t i2cCom1_UserData = { .command = CMDI2C_NONE, .state = STI2C_IDLE, };
static volatile i2c_status_reg_u status = { .byte = 0 };
static volatile i2c_block_t block;
//static volatile i2c_image_t load;
//static volatile i2c_image_t save;
static volatile uint32_t i_flag;
static uint8_t tx_buffer[8];
static uint8_t start_of_flash[PGM_SIZE_BYTE];
static bool write_on_reset = false;
//static konekt_boot_flags_t boot_flags;

static void handle_command(void)
{
    switch(i2cCom1_UserData.command)
    {
    case CMDI2C_READ_STATUS:
        i2cCom1_UserData.state = STI2C_TX;
        tx_buffer[0] = status.byte;
        status.byte &= ~0x02; //clear error bit
        i2cCom1_SlaveState.txBuff = tx_buffer;
        i2cCom1_SlaveState.txSize = 1;
        break;
    case CMDI2C_SYSTEMBOOT_VERSION:
        i2cCom1_UserData.state = STI2C_TX;
        i2cCom1_SlaveState.txBuff = (const uint8_t*)&id.major;
        i2cCom1_SlaveState.txSize = 3;
        break;
    case CMDI2C_SYSTEMFIRMWARE_VERSION:
        i2cCom1_UserData.state = STI2C_TX;
        if(*(uint32_t*)0x000060CC == HOLO)
            i2cCom1_SlaveState.txBuff = (const uint8_t*)0x000060C8;
        else
        {
            memset(tx_buffer, 0, 8);
            i2cCom1_SlaveState.txBuff = tx_buffer;
        }
        i2cCom1_SlaveState.txSize = 3;
        break;
    case CMDI2C_WRITE_SYSTEM_BLOCK:
    //case CMDI2C_WRITE_EXTERNAL:
        if(status.fields.busy)
        {
            i2cCom1_UserData.state = STI2C_IDLE;
            i2cCom1_SlaveState.rxSize = 0;
        }
        else
        {
            i2cCom1_UserData.state = STI2C_RX;
            i2cCom1_SlaveState.rxBuff = (uint8_t*) &block;
            i2cCom1_SlaveState.rxSize = sizeof(block);
        }
        break;
    case CMDI2C_RESET:
        i2cCom1_UserData.state = STI2C_IDLE;
        i_flag = FLAG_RESET;
        break;
    case CMDI2C_USER_NOTIFY:
        i2cCom1_UserData.state = STI2C_IDLE;
        i_flag = FLAG_USER_NOTIFY;
        break;
    }
}

static void handle_receive(void)
{
    switch(i2cCom1_UserData.command)
    {
    case CMDI2C_WRITE_SYSTEM_BLOCK:
        status.fields.busy = 1; //no more writes until this one completes
        i2cCom1_UserData.state = STI2C_IDLE;
        i_flag = FLAG_WRITE_SYSTEM;
        break;
//    case CMDI2C_WRITE_EXTERNAL:
//        status.fields.busy = 1;
//        i2cCom1_UserData.state = STI2C_IDLE;
//        i_flag = FLAG_WRITE_EXTERNAL;
//        break;
    }
}

void i2cCom1_Handler(i2c_slave_event_t slaveEvent)
{
    switch(slaveEvent)
    {
    // Transmit request
    case kI2CSlaveTxReq:
        if(i2cCom1_UserData.state == STI2C_TX)
            i2cCom1_SlaveState.isTxBusy = true;
        else
        {
            i2cCom1_UserData.state = STI2C_IDLE;
            i2cCom1_SlaveState.txSize = 0;
            i2cCom1_SlaveState.rxSize = 0;
        }
        break;

        // Receive request
    case kI2CSlaveRxReq:
        i2cCom1_UserData.state = STI2C_CMD;
        i2cCom1_SlaveState.rxBuff = &i2cCom1_UserData.command;
        i2cCom1_SlaveState.rxSize = 1;
        i2cCom1_SlaveState.txSize = 0;
        i2cCom1_SlaveState.isRxBusy = true;
        break;

        // Transmit buffer is empty
    case kI2CSlaveTxEmpty:
        i2cCom1_UserData.state = STI2C_IDLE;
        i2cCom1_SlaveState.isTxBusy = false;
        break;

        // Receive buffer is full
    case kI2CSlaveRxFull:
        if(i2cCom1_UserData.state == STI2C_CMD)
            handle_command();
        else
            handle_receive();
        break;

    default:
        break;
    }
}

void i2cCom1_Task(void)
{
    uint32_t next_toggle = OSA_TimeGetMsec();
    uint32_t toggle_count = 0;

    GPIO_DRV_SetPinDir(M1_RESET, kGpioDigitalOutput);
    GPIO_DRV_ClearPinOutput(M1_RESET);
    GPIO_DRV_SetPinOutput(WAKE_M1);
    GPIO_DRV_SetPinDir(WAKE_M2, kGpioDigitalOutput);
    GPIO_DRV_ClearPinOutput(WAKE_M2);
    OSA_TimeDelay(10);
    GPIO_DRV_SetPinDir(M1_RESET, kGpioDigitalInput);
    OSA_TimeDelay(100);
    GPIO_DRV_SetPinDir(WAKE_M2, kGpioDigitalInput);

    FLASH_init_ram();

    //memset(&boot_flags, 0xFF, sizeof(boot_flags));

    for(;;)
    {
        if(next_toggle < OSA_TimeGetMsec())
        {
            next_toggle += 100;
            GPIO_DRV_WritePinOutput(WAKE_M1,
                    toggle_count == 4 || toggle_count == 7);
            toggle_count++;
            if(toggle_count >= 10) toggle_count = 0;
        }

        uint32_t flag = 0;
        i2c_status_reg_u result = { .byte = 0 };

        if(i_flag)
        {
            INT_SYS_DisableIRQ(I2C0_IRQn);
            flag = i_flag;
            i_flag = 0;
            INT_SYS_EnableIRQ(I2C0_IRQn);

            if(flag == FLAG_USER_NOTIFY)
            {
            }
            if(flag == FLAG_WRITE_SYSTEM)
            {
                //write the block to the internal flash
                result.fields.error = 1;
                uint32_t address = (((block.block_hi << 8) | block.block_low)
                        * 1024) + SYSTEM_APP_ADDRESS;
                if(FLASH_erase_sector(address))
                {
                    if(block.block_hi == 0 && block.block_low == 0)
                    {
                        //skip the first write of the first block until finished
                        memcpy(&start_of_flash, (uint8_t*) block.block,
                                PGM_SIZE_BYTE);
                        if(FLASH_write_block(address + PGM_SIZE_BYTE,
                                ((uint8_t*) block.block) + PGM_SIZE_BYTE,
                                1024 - PGM_SIZE_BYTE))
                        {
                            result.fields.error = 0;
                            write_on_reset = true;
                        }
                    }
                    else if(FLASH_write_block(address, (uint8_t*) block.block,
                            1024))
                    {
                        result.fields.error = 0;
                    }
                }
            }
//            if(flag == FLAG_SETUP_WRITE_EXTERNAL)
//            {
//                //start ublox write
//                result.fields.error = BOOT_SetupUbloxWrite(load.filename, load.size);
//            }
//            if(flag == FLAG_WRITE_EXTERNAL)
//            {
//                //continue write to ublox (not spi)
//                result.fields.error = BOOT_ContinueUbloxWrite((uint8_t*)block.block, load.size < 1024 ? load.size : 1024);
//            }

            INT_SYS_DisableIRQ(I2C0_IRQn);
            status.byte = result.byte;
            INT_SYS_EnableIRQ(I2C0_IRQn);

            GPIO_DRV_ClearPinOutput(M1_EZPCS);
            OSA_TimeDelay(1);

            if(flag == FLAG_RESET)
            {
                if(write_on_reset)
                    FLASH_write_block(SYSTEM_APP_ADDRESS, start_of_flash, PGM_SIZE_BYTE);
//                if(boot_flags.special_code == 0)
//                {
//                    BOOT_WriteFlags(&boot_flags, BOOT_SPECIAL_UBLOX);
//                }
                OSA_TimeDelay(10);
                NVIC_SystemReset();
            }

            GPIO_DRV_SetPinOutput(M1_EZPCS);
        }
    }
}
