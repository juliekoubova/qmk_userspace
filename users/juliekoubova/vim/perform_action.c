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

static uint16_t command_mods  = QK_LCTL;
static uint16_t word_mods     = QK_LCTL;
static uint8_t  line_mods     = 0;
static uint16_t document_mods = QK_LCTL;

static uint8_t document_start = KC_HOME;
static uint8_t document_end   = KC_END;

static uint8_t line_start = KC_HOME;
static uint8_t line_end   = KC_END;

typedef enum { VLINE_DOWN_ASSUMED, VLINE_DOWN, VLINE_UP } vline_t;

static vline_t vline = VLINE_DOWN_ASSUMED;

// if we start vline mode going up, we start the selection from the end of the
// line and select all the way to the start of line after each key release. that
// way, when we get to the beginning of the document, we select the first line
// completely. and vice versa for going down.
// if we first go in one direction, then decide to go in the other and
// overshoot the original starting point, this will stop working, but hey, at
// least we tried.
static void vim_vline_start(vline_t direction) {
    uint8_t first  = direction == VLINE_UP ? line_end : line_start;
    uint8_t second = direction == VLINE_UP ? line_start : line_end;
    vim_send(line_mods | first, VIM_SEND_TAP);
    vim_send(QK_LSFT | line_mods | second, VIM_SEND_TAP);
    vline = direction;
}

void vim_vline_entered(void) {
    // begin vline mode assuming we're gonna go down
    vim_vline_start(VLINE_DOWN_ASSUMED);
}

void vim_set_apple(bool apple) {
    VIM_DPRINTF("apple=%d\n", apple);
    command_mods = apple ? QK_LGUI : QK_LCTL;
    word_mods    = apple ? QK_LALT : QK_LCTL;

    document_mods  = apple ? QK_LGUI : QK_LCTL;
    document_start = apple ? KC_UP : KC_HOME;
    document_end   = apple ? KC_DOWN : KC_END;

    line_mods  = apple ? QK_LGUI : 0;
    line_start = apple ? KC_LEFT : KC_HOME;
    line_end   = apple ? KC_RIGHT : KC_END;
}

static void vim_send_repeated_multi(int8_t repeat, const uint16_t* code16s, uint8_t code16_count) {
    while (repeat > 0) {
        for (int i = 0; i < code16_count; i++) {
            vim_send(code16s[i], VIM_SEND_TAP);
        }
        repeat--;
    }
}

static void vim_send_repeated(int8_t repeat, uint16_t code16, vim_send_type_t type) {
    if (type == VIM_SEND_RELEASE) {
        vim_send(code16, type);
        return;
    }
    while (repeat > 1) {
        vim_send(code16, VIM_SEND_TAP);
        repeat--;
    }
    vim_send(code16, type);
}

void vim_perform_action(vim_action_t action, vim_send_type_t type) {
    vim_pending_t pending = vim_clear_pending();
    switch (action & VIM_MASK_ACTION) {
        case VIM_ACTION_PASTE:
            vim_send_repeated(pending.repeat, command_mods | KC_V, type);
            return;
        case VIM_ACTION_UNDO:
            vim_send_repeated(pending.repeat, command_mods | KC_Z, type);
            return;
        case VIM_ACTION_OPEN_LINE_DOWN:
            vim_send(line_mods | line_end, VIM_SEND_TAP);
            vim_send(KC_ENTER, VIM_SEND_TAP);
            vim_enter_insert_mode();
            return;
        case VIM_ACTION_OPEN_LINE_UP:
            vim_send(line_mods | line_start, VIM_SEND_TAP);
            vim_send(KC_ENTER, VIM_SEND_TAP);
            vim_send(KC_UP, VIM_SEND_TAP);
            vim_enter_insert_mode();
            return;
        case VIM_ACTION_JOIN_LINE:
            vim_send(line_mods | line_end, VIM_SEND_TAP);
            vim_send(KC_DEL, VIM_SEND_TAP);
            return;
        default:
            break;
    }

    uint16_t code16     = KC_NO;
    vline_t  next_vline = VLINE_DOWN_ASSUMED;

    if (action == (VIM_MOD_DELETE | VIM_ACTION_LEFT)) {
        code16 = KC_BSPC;
        action &= ~VIM_MOD_DELETE;
    } else if (action == (VIM_MOD_DELETE | VIM_ACTION_RIGHT)) {
        code16 = KC_DEL;
        action &= ~VIM_MOD_DELETE;
    } else {
        switch (action & VIM_MASK_ACTION) {
            case VIM_ACTION_LEFT:
                code16 = KC_LEFT;
                break;
            case VIM_ACTION_DOWN:
                code16     = KC_DOWN;
                next_vline = VLINE_DOWN;
                break;
            case VIM_ACTION_UP:
                code16     = KC_UP;
                next_vline = VLINE_UP;
                break;
            case VIM_ACTION_RIGHT:
                code16 = KC_RIGHT;
                break;
            case VIM_ACTION_LINE_START:
                code16 = line_mods | line_start;
                break;
            case VIM_ACTION_LINE_END:
                code16 = line_mods | line_end;
                break;
            case VIM_ACTION_WORD_START:
                code16 = word_mods | KC_LEFT;
                break;
            case VIM_ACTION_WORD_END:
                code16 = word_mods | KC_RIGHT;
                break;
            case VIM_ACTION_DOCUMENT_START:
                code16     = document_mods | document_start;
                next_vline = VLINE_UP;
                break;
            case VIM_ACTION_DOCUMENT_END:
                code16     = document_mods | document_end;
                next_vline = VLINE_DOWN;
                break;
            case VIM_ACTION_PAGE_UP:
                code16     = KC_PAGE_UP;
                next_vline = VLINE_UP;
                break;
            case VIM_ACTION_PAGE_DOWN:
                code16     = KC_PAGE_DOWN;
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
        code16 |= QK_LSFT;
        type = VIM_SEND_TAP;
    }

    if (action & VIM_MOD_SELECT) {
        code16 |= QK_LSFT;
    }

    if (vim_get_mode() == VIM_MODE_VLINE) {
        // if we start moving up, re-select the first line, which has been
        // selected with the (now disproven) assumption that we're gonna go down
        if (vline == VLINE_DOWN_ASSUMED && next_vline != VLINE_DOWN_ASSUMED) {
            if (next_vline == VLINE_UP) {
                vim_vline_start(VLINE_UP);
            }
            vline = next_vline;
        }
    }

    if ((action & VIM_MASK_ACTION) == VIM_ACTION_LINE) {
        type = VIM_SEND_TAP;
        vim_send(line_mods | line_start, type);
        vim_send(QK_LSFT | line_mods | line_end, type);
        vim_send(QK_LSFT | KC_RIGHT, type);
        if (pending.repeat > 1) {
            const uint16_t end_left[] = {QK_LSFT | KC_DOWN, QK_LSFT | line_mods | line_end};
            vim_send_repeated_multi(pending.repeat - 1, end_left, 2);
            pending.repeat = 0;
        }
    }

    if (code16 != KC_NO) {
        // keycode is KC_NO in visual mode, where the object is the visual
        // selection; send shifted action as a tap
        vim_send_repeated(pending.repeat, code16, type);
    }

    if (vim_get_mode() == VIM_MODE_VLINE) {
        // select the full line after we release the up/down key, or after a tap
        if (type == VIM_SEND_RELEASE || type == VIM_SEND_TAP) {
            if (vline == VLINE_UP) {
                vim_send(QK_LSFT | line_mods | line_start, VIM_SEND_TAP);
            } else if (vline == VLINE_DOWN) {
                vim_send(QK_LSFT | line_mods | line_end, VIM_SEND_TAP);
            }
        }
    }

    if (action & VIM_MOD_DELETE) {
        vim_send(command_mods | KC_X, VIM_SEND_TAP);
    } else if (action & VIM_MOD_YANK) {
        vim_send(command_mods | KC_C, VIM_SEND_TAP);
    }

    VIM_DPRINTF("vim_perform_action %x\n", action);
    vim_enter_mode(VIM_MODE_FROM_ACTION(action));
}
