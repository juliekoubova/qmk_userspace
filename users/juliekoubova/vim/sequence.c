/* Copyright 2024 (c) Julie Koubova (julie@koubova.net)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sequence.h"
#include <stddef.h>
#include <stdint.h>

#define VIM_SEQUENCE_MAX 512

typedef struct {
    int8_t   repeat;
    size_t   current;
    size_t   count;
    uint16_t codes[VIM_SEQUENCE_MAX];
} vim_sequence_t;

static vim_sequence_t sequence = {};

void vim_queue_sequence(const uint16_t* codes, size_t count, int8_t repeat) {
    if (count > VIM_SEQUENCE_MAX) {
        count = 0;
    }
    sequence.repeat = repeat;
    sequence.current = 0;
    sequence.count = count;
    if (count > 0) {
    }
}

