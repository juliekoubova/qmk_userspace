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
#include "quantum/keycode.h"
#include "platforms/timer.h"
#include "statemachine.h"
#include "vim_mode.h"
#include "vim_send.h"
#include <stdbool.h>

static uint8_t command_mods = MOD_LCTL;
static uint8_t word_mods    = MOD_LCTL;

static uint8_t document_mods  = MOD_LCTL;
static uint8_t document_start = KC_HOME;
static uint8_t document_end   = KC_END;

static uint8_t line_mods  = 0;
static uint8_t line_start = KC_HOME;
static uint8_t line_end   = KC_END;

typedef enum { VLINE_NONE, VLINE_UP, VLINE_DOWN } vline_t;

static vline_t  vline      = VLINE_NONE;
static uint16_t vline_time = 0;

#ifndef VIM_VLINE_TIMEOUT
#    define VIM_VLINE_TIMEOUT 700
#endif

void vim_vline_entered(void) {
    vline = VLINE_NONE;
    vline_time = timer_read();
}

static void vim_vline_start(vline_t direction) {
    uint8_t first  = direction == VLINE_UP ? line_end : line_start;
    uint8_t second = direction == VLINE_UP ? line_start : line_end;
    vim_send(line_mods, first, VIM_SEND_TAP);
    vim_send(MOD_LSFT | line_mods, second, VIM_SEND_TAP);
    vline = direction;
}

void vim_vline_task(void) {
    if (vim_get_mode() == VIM_MODE_VLINE && vline == VLINE_NONE) {
        if (timer_elapsed(vline_time) > VIM_VLINE_TIMEOUT) {
            vim_vline_start(VLINE_DOWN);
        }
    }
}

void vim_set_apple(bool apple) {
    VIM_DPRINTF("apple=%d\n", apple);
    command_mods = apple ? MOD_LGUI : MOD_LCTL;
    word_mods    = apple ? MOD_LALT : MOD_LCTL;

    document_mods  = apple ? MOD_LGUI : MOD_LCTL;
    document_start = apple ? KC_UP : KC_HOME;
    document_end   = apple ? KC_DOWN : KC_END;

    line_mods  = apple ? MOD_LGUI : 0;
    line_start = apple ? KC_LEFT : KC_HOME;
    line_end   = apple ? KC_RIGHT : KC_END;
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
            vim_send_repeated(pending.repeat, command_mods, KC_V, type);
            return;
        case VIM_ACTION_UNDO:
            vim_send_repeated(pending.repeat, command_mods, KC_Z, type);
            return;
        case VIM_ACTION_OPEN_LINE_DOWN:
            vim_send(line_mods, line_end, VIM_SEND_TAP);
            vim_send(0, KC_ENTER, VIM_SEND_TAP);
            vim_enter_insert_mode();
            return;
        case VIM_ACTION_OPEN_LINE_UP:
            vim_send(line_mods, line_start, VIM_SEND_TAP);
            vim_send(0, KC_ENTER, VIM_SEND_TAP);
            vim_send(0, KC_UP, VIM_SEND_TAP);
            vim_enter_insert_mode();
            return;
        case VIM_ACTION_JOIN_LINE:
            vim_send(line_mods, line_end, VIM_SEND_TAP);
            vim_send(0, KC_DEL, VIM_SEND_TAP);
            return;
        case VIM_ACTION_LINE:
            vim_send(line_mods, line_start, VIM_SEND_TAP);
            type = VIM_SEND_TAP;
            action &= ~VIM_MASK_ACTION;
            action |= VIM_ACTION_LINE_END;
            break;
        default:
            break;
    }

    uint16_t keycode    = KC_NO;
    uint8_t  mods       = 0;
    vline_t  next_vline = VLINE_NONE;

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
                keycode    = KC_DOWN;
                next_vline = VLINE_DOWN;
                break;
            case VIM_ACTION_UP:
                keycode    = KC_UP;
                next_vline = VLINE_UP;
                break;
            case VIM_ACTION_RIGHT:
                keycode = KC_RIGHT;
                break;
            case VIM_ACTION_LINE_START:
                keycode = line_start;
                mods    = line_mods;
                break;
            case VIM_ACTION_LINE_END:
                keycode = line_end;
                mods    = line_mods;
                break;
            case VIM_ACTION_WORD_START:
                keycode = KC_LEFT;
                mods    = word_mods;
                break;
            case VIM_ACTION_WORD_END:
                keycode = KC_RIGHT;
                mods    = word_mods;
                break;
            case VIM_ACTION_DOCUMENT_START:
                keycode    = document_start;
                mods       = document_mods;
                next_vline = VLINE_UP;
                break;
            case VIM_ACTION_DOCUMENT_END:
                keycode    = document_end;
                mods       = document_mods;
                next_vline = VLINE_DOWN;
                break;
            case VIM_ACTION_PAGE_UP:
                keycode    = KC_PAGE_UP;
                next_vline = VLINE_UP;
                break;
            case VIM_ACTION_PAGE_DOWN:
                keycode    = KC_PAGE_DOWN;
                next_vline = VLINE_DOWN;
                break;
            default:
                break;
        }
    }

    switch (pending.keycode) {
        case KC_C:
            action |= VIM_MOD_DELETE | VIM_ENTER_INSERT;
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

    if (action & (VIM_MOD_DELETE | VIM_MOD_YANK)) {
        mods |= MOD_LSFT;
        type = VIM_SEND_TAP;
    }

    if (action & VIM_MOD_SELECT) {
        mods |= MOD_LSFT;
    }

    if (vim_get_mode() == VIM_MODE_VLINE) {
        // if we are still in VLINE_NONE, begin the full line selection based
        // on the direction we're going
        if (vline == VLINE_NONE && next_vline != VLINE_NONE) {
            vim_vline_start(next_vline);
        }
    }

    if (keycode != KC_NO) {
        // keycode is KC_NO in visual mode, where the object is the visual
        // selection; send shifted action as a tap
        vim_send_repeated(pending.repeat, mods, keycode, type);
    }

    if (vim_get_mode() == VIM_MODE_VLINE) {
        if (type == VIM_SEND_RELEASE || type == VIM_SEND_TAP) {
            if (vline == VLINE_UP) {
                vim_send(MOD_LSFT | line_mods, line_start, VIM_SEND_TAP);
            } else if (vline == VLINE_DOWN) {
                vim_send(MOD_LSFT | line_mods, line_end, VIM_SEND_TAP);
            }
        }
    }

    if (action & VIM_MOD_DELETE) {
        vim_send(command_mods, KC_X, VIM_SEND_TAP);
    } else if (action & VIM_MOD_YANK) {
        vim_send(command_mods, KC_C, VIM_SEND_TAP);
    }

    VIM_DPRINTF("vim_perform_action %x\n", action);
    vim_enter_mode(VIM_MODE_FROM_ACTION(action));
}
