/*
  flash.h - write to internal flash

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

#ifndef SOURCES_FLASH_H_
#define SOURCES_FLASH_H_

#include "Cpu.h"

void FLASH_init_ram(void);
bool FLASH_erase_sector(uint32_t sector_address);
bool FLASH_write_block(uint32_t address, uint8_t *block, uint32_t size);

#endif /* SOURCES_FLASH_H_ */
