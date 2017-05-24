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

#ifndef SOURCES_JUMP_H_
#define SOURCES_JUMP_H_

typedef struct
{
    uint32_t packet_id;
    uint32_t block_id;
    uint32_t pad[14];
}update_record_t;

typedef struct
{
    update_record_t record;
    uint8_t block[1024];
}update_packet_t;

bool JUMP_IsValid(void);
void JUMP_ToApp(void);

#endif /* SOURCES_JUMP_H_ */
