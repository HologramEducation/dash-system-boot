/*
  jump.h - jump to new application starting point

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
#include "Cpu.h"
#include "boot.h"

__attribute__( ( always_inline ) ) __STATIC_INLINE void __set_PC(uint32_t resetHandler)
{
    __ASM volatile ("MOV pc, %0\n" : : "r" (resetHandler) : "pc");
}

bool JUMP_IsValid(void)
{
    uint32_t *pSP = (uint32_t*)(SYSTEM_APP_ADDRESS);
    uint32_t *pPC = (uint32_t*)(SYSTEM_APP_ADDRESS+4);

    return (*pSP > 0x1FFFE000 && *pSP <= 0x20006000 && *pPC > SYSTEM_APP_ADDRESS && *pPC < 0x00040000);
}

void JUMP_ToApp(void)
{
    if(JUMP_IsValid())
    {
        uint32_t *pSP = (uint32_t*)(SYSTEM_APP_ADDRESS);
        uint32_t *pPC = (uint32_t*)(SYSTEM_APP_ADDRESS+4);
        __disable_irq();
        __DMB();
        SCB->VTOR = SYSTEM_APP_ADDRESS;
        __DSB();
        __set_MSP(*pSP);
        __set_PC(*pPC);
    }
}
