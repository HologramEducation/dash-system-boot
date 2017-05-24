/*
  ring.c - ring buffer

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

#include "ring.h"
#include "Cpu.h"

uint32_t next_head(ring_t *ring)
{
    return (uint32_t)(ring->head + 1) % ring->size;
}

uint32_t next_tail(ring_t *ring)
{
    return (uint32_t)(ring->tail + 1) % ring->size;
}

void RING_init(ring_t *ring, uint8_t **buffer, uint32_t size)
{
    ring->buffer = *buffer;
    ring->size = size;
    ring->head = 0;
    ring->tail = 0;
}

bool RING_push(ring_t *ring, uint8_t b)
{
    uint32_t i = next_head(ring);

    if(i != ring->tail)
    {
        ring->buffer[ring->head] = b;
        ring->head = i;
        return true;
    }
    return false;
}

void RING_flush(ring_t *ring)
{
    ring->head = 0;
    ring->tail = 0;
}

int32_t RING_pop(ring_t *ring)
{
    if(ring->tail == ring->head)
        return -1;

    uint8_t value = ring->buffer[ring->tail];
    ring->tail = next_tail(ring);

    return value;
}

uint32_t RING_available(ring_t *ring)
{
    int delta = ring->head - ring->tail;

    if(delta < 0)
        return ring->size + delta;
    else
        return delta;
}

uint8_t RING_peek(ring_t *ring)
{
    if(ring->tail == ring->head)
        return -1;

    return ring->buffer[ring->tail];
}

bool RING_isFull(ring_t *ring)
{
    return (next_head(ring) == ring->tail);
}

bool RING_find_string(ring_t *ring, const char *match, uint32_t timeout_ms)
{
    if(*match == 0) return true;

    uint8_t c = 0;
    const char *comp = match;
    uint32_t start = OSA_TimeGetMsec();
    bool seeking = true;

    while(OSA_TimeGetMsec() - start < timeout_ms) {
        //look for the first char
        if(seeking) {
            if(!RING_get_until(ring, NULL, *match, timeout_ms - (OSA_TimeGetMsec() - start)))
                return false;
            else
            {
                seeking = false;
                comp = &match[1];
                if(*comp == 0) return true;
            }
        }
        // looking for next char
        else {
            while(RING_available(ring)) {
                c = (uint8_t)RING_peek(ring);
                if(c == *comp++) {
                    RING_pop(ring);
                    if(*comp == 0)
                        return true;
                } else {
                    seeking = true;
                    break;
                }
            }
        }
    }
    return false;
}

bool RING_get_until(ring_t *ring, char *buffer, char delim, uint32_t timeout_ms)
{
    char c = 0;
    char *p = buffer;
    uint32_t start = OSA_TimeGetMsec();

    while(OSA_TimeGetMsec() - start < timeout_ms) {
        if(RING_available(ring)) {
            c = (char)RING_pop(ring);
            if(c == delim)
                return true;
            if(buffer)
                *p++ = c;
        }
    }
    return false;
}

uint32_t RING_get(ring_t *ring, char* buffer, uint32_t count, uint32_t timeout_ms)
{
    uint32_t read = 0;
    uint32_t start = OSA_TimeGetMsec();
    while((OSA_TimeGetMsec() - start < timeout_ms) && (read < count)) {
        if(RING_available(ring)) {
            buffer[read++] = (char)RING_pop(ring);
        }
    }
    return read;
}
