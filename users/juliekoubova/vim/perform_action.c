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

#include "debug.h"
#include "pending.h"
#include "perform_action.h"
#include "quantum/keycode.h"
#include "statemachine.h"
#include "vim_send.h"
#include <stdbool.h>

static uint8_t command_mod      = MOD_LCTL;
static uint8_t document_nav_mod = MOD_LCTL;
static uint8_t word_nav_mod     = MOD_LCTL;

void vim_set_apple(bool apple) {
    VIM_DPRINTF("apple=%d\n", apple);
    command_mod      = apple ? MOD_LGUI : MOD_LCTL;
    document_nav_mod = apple ? MOD_LGUI : MOD_LCTL;
    word_nav_mod     = apple ? MOD_LALT : MOD_LCTL;
}

static void vim_send_repeated(int8_t repeat, uint8_t mods, uint16_t keycode, vim_send_type_t type) {
    if (type == VIM_SEND_RELEASE) {
        vim_send(mods, keycode, type);
        return;
    }
    while (repeat > 1) {
        vim_send(mods, keycode, VIM_SEND_TAP);
        repeat--;
    }
    vim_send(mods, keycode, type);
}

void vim_perform_action(vim_action_t action, vim_send_type_t type) {
    vim_pending_t pending = vim_clear_pending();
    switch (action & VIM_MASK_ACTION) {
        case VIM_ACTION_PASTE:
            vim_send_repeated(pending.repeat, command_mod, KC_V, type);
            return;
        case VIM_ACTION_UNDO:
            vim_send_repeated(pending.repeat, command_mod, KC_Z, type);
            return;
        case VIM_ACTION_OPEN_LINE_DOWN:
            vim_send(0, KC_END, VIM_SEND_TAP);
            vim_send(0, KC_ENTER, VIM_SEND_TAP);
            return;
        case VIM_ACTION_OPEN_LINE_UP:
            vim_send(0, KC_HOME, VIM_SEND_TAP);
            vim_send(0, KC_ENTER, VIM_SEND_TAP);
            vim_send(0, KC_UP, VIM_SEND_TAP);
            return;
        case VIM_ACTION_JOIN_LINE:
            vim_send(0, KC_END, VIM_SEND_TAP);
            vim_send(0, KC_DEL, VIM_SEND_TAP);
            return;
        case VIM_ACTION_LINE:
            vim_send(0, KC_HOME, VIM_SEND_TAP);
            type   = VIM_SEND_TAP;
            action = (action & VIM_MASK_MOD) | VIM_ACTION_LINE_END;
            break;
        default:
            break;
    }

    uint16_t keycode = KC_NO;
    uint8_t  mods    = 0;

    if (action == (VIM_MOD_DELETE | VIM_ACTION_LEFT)) {
        keycode = KC_BSPC;
        action &= ~VIM_MOD_DELETE;
    } else if (action == (VIM_MOD_DELETE | VIM_ACTION_RIGHT)) {
        keycode = KC_DEL;
        action &= ~VIM_MOD_DELETE;
    } else {
        switch (action & VIM_MASK_ACTION) {
            case VIM_ACTION_LEFT:
                keycode = KC_LEFT;
                break;
            case VIM_ACTION_DOWN:
                keycode = KC_DOWN;
                break;
            case VIM_ACTION_UP:
                keycode = KC_UP;
                break;
            case VIM_ACTION_RIGHT:
                keycode = KC_RIGHT;
                break;
            case VIM_ACTION_LINE_START:
                keycode = KC_HOME;
                break;
            case VIM_ACTION_LINE_END:
                keycode = KC_END;
                break;
            case VIM_ACTION_WORD_START:
                keycode = KC_LEFT;
                mods    = word_nav_mod;
                break;
            case VIM_ACTION_WORD_END:
                keycode = KC_RIGHT;
                mods    = word_nav_mod;
                break;
            case VIM_ACTION_DOCUMENT_START:
                keycode = KC_HOME;
                mods    = document_nav_mod;
                break;
            case VIM_ACTION_DOCUMENT_END:
                keycode = KC_END;
                mods    = document_nav_mod;
                break;
            case VIM_ACTION_PAGE_UP:
                keycode = KC_PAGE_UP;
                break;
            case VIM_ACTION_PAGE_DOWN:
                keycode = KC_PAGE_DOWN;
                break;
            default:
                break;
        }
    }

    switch (pending.keycode) {
        case KC_C:
            action |= VIM_MOD_CHANGE;
            type = VIM_SEND_TAP;
            break;
        case KC_D:
            action |= VIM_MOD_DELETE;
            type = VIM_SEND_TAP;
            break;
        case KC_Y:
            action |= VIM_MOD_YANK;
            type = VIM_SEND_TAP;
            break;
        default:
            break;
    }

    if (action & (VIM_MOD_CHANGE | VIM_MOD_YANK)) {
        mods |= MOD_LSFT;
        type = VIM_SEND_TAP;
    }

    if (action & VIM_MOD_SELECT) {
        mods |= MOD_LSFT;
    }

    if (keycode != KC_NO) {
        // keycode is KC_NO in visual mode, where the object is the visual
        // selection; send shifted action as a tap
        vim_send_repeated(pending.repeat, mods, keycode, type);
    }

    if (action & (VIM_MOD_DELETE | VIM_MOD_CHANGE)) {
        vim_send(command_mod, KC_X, VIM_SEND_TAP);
    } else if (action & VIM_MOD_YANK) {
        vim_send(command_mod, KC_C, VIM_SEND_TAP);
    }

    if (action & (VIM_MOD_CHANGE | VIM_MOD_INSERT_AFTER)) {
        vim_enter_insert_mode();
    } else if (action & VIM_MOD_VISUAL_AFTER) {
        vim_enter_visual_mode();
    } else if (action & VIM_MOD_COMMAND_AFTER) {
        vim_enter_command_mode();
    }
}
