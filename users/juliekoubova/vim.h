/* Copyright 2023 (c) Julie Koubova (julie@koubova.net)
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
#include <stdbool.h>
#include <stdint.h>
#include "quantum/quantum.h"
#include "vim/vim_mode.h"

bool process_record_vim(uint16_t keycode, const keyrecord_t *record, uint16_t vim_keycode);
bool vim_is_active_key(uint16_t keycode);
void vim_set_apple(bool apple);
void vim_task(void);
