/*
  ring.h - ring buffer

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

#ifndef SOURCES_RING_H_
#define SOURCES_RING_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t *buffer;
    uint32_t size;
    volatile int32_t head;
    volatile int32_t tail;
}ring_t;

void RING_init(ring_t *ring, uint8_t **buffer, uint32_t size);
bool RING_push(ring_t *ring, uint8_t b);
void RING_flush(ring_t *ring);
int32_t RING_pop(ring_t *ring);
uint32_t RING_available(ring_t *ring);
uint8_t RING_peek(ring_t *ring);
bool RING_isFull(ring_t *ring);
bool RING_find_string(ring_t *ring, const char *match, uint32_t timeout_ms);
bool RING_get_until(ring_t *ring, char *buffer, char delim, uint32_t timeout_ms);
uint32_t RING_get(ring_t *ring, char* buffer, uint32_t count, uint32_t timeout_ms);

#endif
