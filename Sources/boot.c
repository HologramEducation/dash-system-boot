/*
  boot.c - handle flash update commands

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
#include <stdio.h>
#include "Cpu.h"
#include "boot.h"
#include "spiComEZPort.h"
#include "flash.h"
#include "ext_flash.h"
#include "gpio1.h"
#include "lpuartUblox.h"

#define BOOT_FLAG_ADDRESS (SYSTEM_APP_ADDRESS - FSL_FEATURE_FLASH_PFLASH_BLOCK_SECTOR_SIZE)
#define BOOT_FLAG_ERASED (0xFFFFFFFF)

#define USER_WRITE_SIZE (16)
#define USER_SECTOR_SIZE (4096)
#define UBLOX_READ_SIZE (32)

#define MAX(a,b) (a>b?a:b)

#define UBLOX_RESET_N GPIO_MAKE_PIN(GPIOA_IDX, 1U)
static const gpio_input_pin_user_config_t ublox_reset_input_config = {
    .pinName = UBLOX_RESET_N,
    .config.isPullEnable = false,
    .config.pullSelect = kPortPullUp,
    .config.isPassiveFilterEnabled = false,
    .config.interrupt = kPortIntDisabled
};



konekt_flash_id_t __attribute__((section (".idSection"))) id __attribute__ ((aligned (4))) = {
    0x544F4F42, //BOOT
    0x324D5044, //DPM2 for DashPro M2
    VERSION_MAJOR,
    VERSION_MINOR,
    VERSION_REVISION,
    0,
    HOLO,
    "konekt.io",
    "Dash",
    "Boot",
    "System",
    "System Bootloader"
};

static uint8_t pgm_buffer[MAX(USER_WRITE_SIZE, FSL_FEATURE_FLASH_PFLASH_BLOCK_SECTOR_SIZE)];
static uint8_t lpuart_ublox_rxbuffer[FSL_FEATURE_FLASH_PFLASH_BLOCK_SECTOR_SIZE*2];
unsigned char ublox_rx[8];

ring_t ublox_ring = {
        .buffer = lpuart_ublox_rxbuffer,
        .size = FSL_FEATURE_FLASH_PFLASH_BLOCK_SECTOR_SIZE*2,
        .head = 0,
        .tail = 0
};

bool BOOT_ublox_echo_off(void)
{
    //ATE0\r
    //wait for
    //u-blox
    RING_flush(&ublox_ring);
    LPUART_DRV_SendDataBlocking(FSL_LPUARTUBLOX, "ATE0\r", 5, 1000);
    if(!RING_find_string(&ublox_ring, "OK", 10000)) return false;
    return true;
}

uint32_t BOOT_ReadFromUblox(const char *filename, uint32_t offset, uint8_t *buffer, uint32_t size)
{
    //AT+URDBLOCK="<filename>",<offset>,<size>\r
    //wait for
    //+URDBLOCK: "<filename>",<size>,"<data>"\r\nOK\r\n
    uint8_t b[8];

    RING_flush(&ublox_ring);

    LPUART_DRV_SendDataBlocking(FSL_LPUARTUBLOX, "AT+URDBLOCK=\"", 13, 1000);
    LPUART_DRV_SendDataBlocking(FSL_LPUARTUBLOX, filename, strlen(filename), 1000);
    LPUART_DRV_SendDataBlocking(FSL_LPUARTUBLOX, "\",", 2, 1000);
    sprintf(b, "%d", offset);
    LPUART_DRV_SendDataBlocking(FSL_LPUARTUBLOX, b, strlen(b), 1000);
    LPUART_DRV_SendDataBlocking(FSL_LPUARTUBLOX, ",", 1, 1000);
    sprintf(b, "%d", size);
    LPUART_DRV_SendDataBlocking(FSL_LPUARTUBLOX, b, strlen(b), 1000);
    LPUART_DRV_SendDataBlocking(FSL_LPUARTUBLOX, "\r", 1, 1000);

    if(!RING_find_string(&ublox_ring, "+URDBLOCK: \"", 10000)) return 0;
    if(!RING_find_string(&ublox_ring, filename, 1000)) return 0;
    if(!RING_find_string(&ublox_ring, "\",", 1000)) return 0;
    if(!RING_get_until(&ublox_ring, b, ',', 1000)) return 0;
    int32_t size_read = strtol(b, NULL, 0);
    if(size_read < 0 || size_read > size) return 0;
    if(!RING_find_string(&ublox_ring, "\"", 1000)) return 0;
    uint32_t actual_read = RING_get(&ublox_ring, pgm_buffer, size_read, 1000);
    if(!RING_find_string(&ublox_ring, "\r\nOK\r\n", 10000)) return 0;

    return actual_read;
}

void BOOT_LoadSystemFromUblox(const char *filename, uint32_t image_size, uint32_t offset)
{
    //write to internal memory from ublox flash
    uint32_t dst = SYSTEM_APP_ADDRESS;
    uint32_t src = offset;
    uint32_t end = dst + image_size;

    while(dst < end)
    {
        if((dst & (FSL_FEATURE_FLASH_PFLASH_BLOCK_SECTOR_SIZE-1)) == 0)
            FLASH_erase_sector(dst);
        uint32_t len = BOOT_ReadFromUblox(filename, src, pgm_buffer, end - dst < UBLOX_READ_SIZE ? end - dst : UBLOX_READ_SIZE);
        if(len == 0) {
            continue;
        }

        FLASH_write_block(dst, pgm_buffer, UBLOX_READ_SIZE);
        dst += len;
        src += len;
    }
}

void BOOT_LoadUserFromUblox(uint32_t dst, const char* filename, uint32_t image_size, uint32_t offset)
{
    //write to internal memory from ublox flash
    uint32_t src = offset;
    uint32_t end = dst + image_size;

    while(dst < end)
    {
        if((dst & (USER_SECTOR_SIZE-1)) == 0)
            EXT_erase_sector(FSL_SPICOMEZPORT, dst);
        uint32_t len = BOOT_ReadFromUblox(filename, src, pgm_buffer, end - dst < UBLOX_READ_SIZE ? end - dst : UBLOX_READ_SIZE);
        if(len == 0) {
            return;
        }

        EXT_write_block(FSL_SPICOMEZPORT, dst, pgm_buffer, UBLOX_READ_SIZE);
        dst += len;
        src += len;
    }
}

void BOOT_LoadSystemFromInternal(uint32_t src, uint32_t size)
{
    //write to internal memory from internal flash
    uint32_t dst = SYSTEM_APP_ADDRESS;
    uint32_t end = dst + size;

    while(dst < end)
    {
        FLASH_erase_sector(dst);
        FLASH_write_block(dst, (uint8_t *)src, FSL_FEATURE_FLASH_PFLASH_BLOCK_SECTOR_SIZE);
        dst += FSL_FEATURE_FLASH_PFLASH_BLOCK_SECTOR_SIZE;
        src += FSL_FEATURE_FLASH_PFLASH_BLOCK_SECTOR_SIZE;
    }
}

static __inline__ void delayMicroseconds( uint32_t usec )
{
  if ( usec == 0 )
  {
    return ;
  }

  /*
   *  The following loop:
   *
   *    for (; ul; ul--) {
   *      __asm__ volatile("");
   *    }
   *
   *  produce the following assembly code:
   *
   *    loop:
   *      subs r3, #1        // 1 Core cycle
   *      bne.n loop         // 1 Core cycle + 1 if branch is taken
   */

  // VARIANT_MCK / 1000000 == cycles needed to delay 1uS
  //                     3 == cycles used in a loop
  uint32_t n = usec * (SystemCoreClock / 1000000) / 3;
  __asm__ __volatile__(
    "1:              \n"
    "   sub %0, #1   \n" // substract 1 from %0 (n)
    "   bne 1b       \n" // if result is not 0 jump to 1
    : "+r" (n)           // '%0' is n variable with RW constraints
    :                    // no input
    :                    // no clobber
  );
  // https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html
  // https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#Volatile
}


void BOOT_CheckFlag(void)
{
    konekt_boot_flags_t *boot_flags = (konekt_boot_flags_t *)BOOT_FLAG_ADDRESS;

    if(boot_flags->special_code == BOOT_SPECIAL_EXT) {
        PE_low_level_init();
        FLASH_init_ram();
        __ISB();
        FLASH_erase_sector(BOOT_FLAG_ADDRESS);
        return;
    }

    if(boot_flags->special_code != BOOT_SPECIAL_UBLOX ||
       boot_flags->end_code != BOOT_SPECIAL_UBLOX)
        return;

    PE_low_level_init();
    FLASH_init_ram();
    __ISB();

    if(boot_flags->internal_system_src != BOOT_FLAG_ERASED &&
       boot_flags->internal_system_size != BOOT_FLAG_ERASED)
    {
        BOOT_LoadSystemFromInternal(boot_flags->internal_system_src, boot_flags->internal_system_size);
    }


    if( (boot_flags->system_size != BOOT_FLAG_ERASED) || (boot_flags->userboot_size != BOOT_FLAG_ERASED) || (boot_flags->user_size != BOOT_FLAG_ERASED))
    {
        uint32_t retry = 3;
        while(!BOOT_ublox_echo_off())
        {
            LPUART_DRV_SendDataBlocking(FSL_LPUARTUBLOX, "\x11", 1, 1000);
            if(--retry == 0) {
                retry = 3;
                GPIO_DRV_InputPinInit(&ublox_reset_input_config);
                GPIO_DRV_ClearPinOutput(UBLOX_RESET_N);
                GPIO_DRV_SetPinDir(UBLOX_RESET_N, kGpioDigitalOutput);
                delayMicroseconds(60);
                GPIO_DRV_SetPinDir(UBLOX_RESET_N, kGpioDigitalInput);
                OSA_TimeDelay(3000);
            }
            OSA_TimeDelay(1000);
        }

        if(boot_flags->system_size != BOOT_FLAG_ERASED)
        {
            BOOT_LoadSystemFromUblox(boot_flags->system_filename, boot_flags->system_size, boot_flags->system_offset);
        }

        if( (boot_flags->userboot_size != BOOT_FLAG_ERASED) || (boot_flags->user_size != BOOT_FLAG_ERASED)) {
            //RESET User module into EZPort
            GPIO_DRV_ClearPinOutput(M1_EZPCS);
            GPIO_DRV_SetPinDir(M1_RESET, kGpioDigitalOutput);
            GPIO_DRV_ClearPinOutput(M1_RESET);
            OSA_TimeDelay(10);
            GPIO_DRV_SetPinOutput(M1_RESET);
            OSA_TimeDelay(10);
            GPIO_DRV_SetPinOutput(M1_EZPCS);

            if(boot_flags->userboot_size != BOOT_FLAG_ERASED)
            {
                BOOT_LoadUserFromUblox(0x0, boot_flags->userboot_filename, boot_flags->userboot_size, boot_flags->userboot_offset);
            }
            if(boot_flags->user_size != BOOT_FLAG_ERASED)
            {
                BOOT_LoadUserFromUblox(USER_APP_ADDRESS, boot_flags->user_filename, boot_flags->user_size, boot_flags->user_offset);
            }

            //RESET User module into run mode
            GPIO_DRV_ClearPinOutput(M1_RESET);
            OSA_TimeDelay(10);
            GPIO_DRV_SetPinDir(M1_RESET, kGpioDigitalInput);
        }
    }

    FLASH_erase_sector(BOOT_FLAG_ADDRESS);

    OSA_TimeDelay(3000);
    NVIC_SystemReset();
}
