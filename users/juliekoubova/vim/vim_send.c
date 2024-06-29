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

#include "vim_send.h"
#include "debug.h"
#include "quantum/quantum.h"

#define VIM_TAP_DELAY 30

void vim_send(uint16_t code16, vim_send_type_t type) {
    uint8_t mods = QK_MODS_GET_MODS(code16);
    uint8_t keycode = QK_MODS_GET_BASIC_KEYCODE(code16);

    if (mods && (type == VIM_SEND_PRESS || type == VIM_SEND_TAP)) {
        VIM_DPRINTF("register mods=%x\n", mods);
        register_mods(mods);
    }

    switch (type) {
        case VIM_SEND_TAP:
            VIM_DPRINTF("tap keycode=%x\n", keycode);
            tap_code_delay(keycode, VIM_TAP_DELAY);
            break;
        case VIM_SEND_PRESS:
            VIM_DPRINTF("register keycode=%x\n", keycode);
            register_code(keycode);
            wait_ms(VIM_TAP_DELAY);
            break;
        case VIM_SEND_RELEASE:
            VIM_DPRINTF("unregister keycode=%x\n", keycode);
            unregister_code(keycode);
            wait_ms(VIM_TAP_DELAY);
            break;
    }

    if (mods && (type == VIM_SEND_RELEASE || type == VIM_SEND_TAP)) {
        VIM_DPRINTF("unregister mods=%x\n", mods);
        unregister_mods(mods);
    }
}

