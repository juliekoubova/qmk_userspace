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
#include "quantum/logging/print.h"

#ifdef VIM_DEBUG
#    define VIM_DPRINT(s) dprint("[vim] " s)
#    define VIM_DPRINTF(...) dprintf("[vim] " __VA_ARGS__)
#else
#    define VIM_DPRINT(s) ((void)0)
#    define VIM_DPRINTF(...) ((void)0)
#endif

