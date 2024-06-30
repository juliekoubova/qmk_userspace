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

#pragma once
#include <stddef.h>
#include <stdint.h>

typedef enum {
    VIM_SEND_NONE    = 0x0,
    VIM_SEND_PRESS   = 0x1,
    VIM_SEND_RELEASE = 0x2,
    VIM_SEND_TAP     = VIM_SEND_PRESS | VIM_SEND_RELEASE
} vim_send_type_t;

void vim_send(uint16_t keycode, vim_send_type_t);
void vim_send_multi(const uint16_t* code16s, size_t count);
void vim_send_repeated(int8_t repeat, uint16_t code16, vim_send_type_t type);
void vim_send_repeated_multi(int8_t repeat, const uint16_t* code16s, uint8_t code16_count);

