/*
  ext_flash.h - write to external flash

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

#ifndef SOURCES_EXT_FLASH_H_
#define SOURCES_EXT_FLASH_H_

#include "Cpu.h"

void EXT_read_block(uint32_t instance, uint32_t address, uint8_t* buffer, uint32_t count);
void EXT_write_block(uint32_t instance, uint32_t address, uint8_t* buffer, uint32_t count);
void EXT_erase_sector(uint32_t instance, uint32_t address);
void EXT_unlock(uint32_t instance);
void EXT_reset(uint32_t instance);



#endif /* SOURCES_EXT_FLASH_H_ */
