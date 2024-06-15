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

#include "debug.h"
#include "pending.h"
#include "perform_action.h"
#include "quantum/quantum.h"
#include "vim_mode.h"
#include "vim_send.h"

static vim_mode_t      vim_mode      = VIM_MODE_INSERT;
static vim_key_state_t vim_key_state = VIM_KEY_NONE;
static uint8_t         vim_mods      = 0; // we modify the actual mods so we can't rely on them

static void vim_cancel_os_selection(void) {
    vim_send(0, KC_LEFT, VIM_SEND_TAP);
}

static void vim_set_mode(vim_mode_t mode) {
    vim_mode = mode;
    vim_mods = mode == VIM_MODE_INSERT ? 0 : get_mods();
    VIM_DPRINTF("entering mode=%d, capturing mods=%x\n", mode, vim_mods);
    vim_clear_pending();
    clear_keyboard();
    layer_state_set(default_layer_state);
    vim_mode_changed(vim_mode);
}

vim_mode_t vim_get_mode(void) {
    return vim_mode;
}

__attribute__((weak)) void vim_mode_changed(vim_mode_t mode) {}

void vim_enter_insert_mode(void) {
    if (vim_mode == VIM_MODE_INSERT) {
        return;
    }
    uint8_t mods = vim_mods;
    VIM_DPRINTF("entering INSERT mode, restoring mods=%x\n", mods);
    vim_set_mode(VIM_MODE_INSERT);
    register_mods(mods);
}

void vim_enter_command_mode(void) {
    switch (vim_mode) {
        case VIM_MODE_COMMAND:
            return;
        case VIM_MODE_VISUAL:
        case VIM_MODE_VLINE:
            vim_cancel_os_selection();
            break;
        default:
            break;
    }
    VIM_DPRINT("entering COMMAND mode\n");
    vim_set_mode(VIM_MODE_COMMAND);
}

void vim_enter_visual_mode(void) {
    if (vim_mode == VIM_MODE_VISUAL) {
        return;
    }
    VIM_DPRINT("entering VISUAL mode\n");
    // don't return to insert after vim key is released
    vim_set_vim_key_state(VIM_KEY_NONE);
    vim_set_mode(VIM_MODE_VISUAL);
}

void vim_enter_vline_mode(void) {
    if (vim_mode == VIM_MODE_VLINE) {
        return;
    }
    VIM_DPRINT("entering V-LINE mode\n");
    // don't return to insert after vim key is released
    vim_set_vim_key_state(VIM_KEY_NONE);
    vim_clear_vline_direction();
    vim_set_mode(VIM_MODE_VLINE);
}

void vim_enter_mode(vim_mode_t mode) {
    VIM_DPRINTF("vim_enter_mode: %d\n", mode);
    switch (mode) {
        case VIM_MODE_INSERT:
            vim_enter_insert_mode();
            break;
        case VIM_MODE_VISUAL:
            vim_enter_visual_mode();
            break;
        case VIM_MODE_VLINE:
            vim_enter_vline_mode();
            break;
        case VIM_MODE_COMMAND:
            vim_enter_command_mode();
            break;
    }
}

void vim_set_mod(uint16_t keycode, bool pressed) {
    uint8_t bit = MOD_BIT(keycode);
    vim_mods    = pressed ? (vim_mods | bit) : (vim_mods & ~bit);
    VIM_DPRINTF("vim_mods = %x\n", vim_mods);
}

uint8_t vim_get_mods(void) {
    return vim_mods;
}

vim_key_state_t vim_get_vim_key_state(void) {
    return vim_key_state;
}

vim_key_state_t vim_set_vim_key_state(vim_key_state_t key_state) {
    vim_key_state_t prev = vim_key_state;
    vim_key_state        = key_state;
    return prev;
}

void vim_dprintf_key(const char *prefix, uint16_t keycode, const keyrecord_t *record) {
    VIM_DPRINTF("%s %s keycode=%x: mode=%d mods=%x vim_key=%x\n", prefix, record->event.pressed ? "pressed" : "released", keycode, vim_mode, vim_mods, vim_key_state);
}
