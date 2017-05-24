/*
  boot.h - handle flash update commands

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

#ifndef SOURCES_BOOT_H_
#define SOURCES_BOOT_H_

#include "ring.h"

typedef struct
{
    uint32_t special_code;              //0x0000
    uint32_t userboot_size;             //0x0004
    uint32_t userboot_offset;           //0x0008
    char     userboot_filename[256];    //0x000C
    uint32_t user_size;                 //0x010C
    uint32_t user_offset;               //0x0110
    char     user_filename[256];        //0x0114
    uint32_t system_size;               //0x0214
    uint32_t system_offset;             //0x0218
    char     system_filename[256];      //0x021C
    uint32_t internal_system_src;       //0x031C
    uint32_t internal_system_size;      //0x0320
    uint32_t end_code;                  //0x0324
}konekt_boot_flags_t;

typedef struct
{
    uint32_t id;
    uint32_t device;
    uint8_t  major;
    uint8_t  minor;
    uint8_t  revision;
    uint8_t  pad;
    uint32_t valid;
    char     manuf[16];
    char     product[16];
    char     role[16];
    char     proc[16];
    char     desc[32];
}konekt_flash_id_t;

extern konekt_flash_id_t id;
extern ring_t ublox_ring;

void BOOT_CheckFlag(void);

#define USER_APP_ADDRESS            0x00008000
#define SYSTEM_APP_ADDRESS          0x00006000

#define HOLO (0x4F4C4F48)

#define BOOT_SPECIAL_EXT   0x746F6F62 //'boot'
#define BOOT_SPECIAL_UBLOX 0x544F4F42 //'BOOT'

#endif /* SOURCES_BOOT_H_ */
