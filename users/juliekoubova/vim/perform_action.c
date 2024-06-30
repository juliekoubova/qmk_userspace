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

static uint16_t command_mods   = QK_LCTL;
static uint16_t word_mods      = QK_LCTL;
static uint16_t document_start = LCTL(KC_HOME);
static uint16_t document_end   = LCTL(KC_END);
static uint16_t line_start     = KC_HOME;
static uint16_t line_end       = KC_END;

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
    vim_send(first, VIM_SEND_TAP);
    vim_send(LSFT(second), VIM_SEND_TAP);
    vline = direction;
}

void vim_vline_entered(void) {
    // begin vline mode assuming we're gonna go down
    vim_vline_start(VLINE_DOWN_ASSUMED);
}

void vim_set_apple(bool apple) {
    VIM_DPRINTF("apple=%d\n", apple);
    command_mods   = apple ? QK_LGUI : QK_LCTL;
    word_mods      = apple ? QK_LALT : QK_LCTL;
    document_start = apple ? LGUI(KC_UP) : LCTL(KC_HOME);
    document_end   = apple ? LGUI(KC_DOWN) : LCTL(KC_END);
    line_start     = apple ? LGUI(KC_LEFT) : KC_HOME;
    line_end       = apple ? LGUI(KC_RIGHT) : KC_END;
}

void vim_perform_action(vim_action_t action, vim_send_type_t type) {
    vim_pending_t pending = vim_clear_pending();
    switch (action & VIM_MASK_ACTION) {
        case VIM_ACTION_OPEN_LINE_DOWN:
            vim_send(line_end, VIM_SEND_TAP);
            vim_send(KC_ENTER, VIM_SEND_TAP);
            vim_enter_insert_mode();
            return;
        case VIM_ACTION_OPEN_LINE_UP:
            vim_send(line_start, VIM_SEND_TAP);
            vim_send(KC_ENTER, VIM_SEND_TAP);
            vim_send(KC_UP, VIM_SEND_TAP);
            vim_enter_insert_mode();
            return;
        default:
            break;
    }

    uint16_t code16[3]         = {KC_NO, KC_NO, KC_NO};
    vline_t  next_vline        = VLINE_DOWN_ASSUMED;
    bool     selection_cleared = false;

    switch (action & VIM_MASK_ACTION) {
        case VIM_ACTION_LEFT:
            if (action & VIM_MOD_DELETE) {
                *code16 = KC_BSPC;
                action &= ~VIM_MOD_DELETE;
                selection_cleared = true;
            } else {
                *code16 = KC_LEFT;
            }
            break;
        case VIM_ACTION_RIGHT:
            if (action & VIM_MOD_DELETE) {
                *code16 = KC_DEL;
                action &= ~VIM_MOD_DELETE;
                selection_cleared = true;
            } else {
                *code16 = KC_RIGHT;
            }
            break;
        case VIM_ACTION_DOWN:
            *code16    = KC_DOWN;
            next_vline = VLINE_DOWN;
            break;
        case VIM_ACTION_UP:
            *code16    = KC_UP;
            next_vline = VLINE_UP;
            break;
        case VIM_ACTION_LINE_START:
            *code16 = line_start;
            break;
        case VIM_ACTION_LINE_END:
            *code16 = line_end;
            break;
        case VIM_ACTION_WORD_START:
            *code16 = word_mods | KC_LEFT;
            break;
        case VIM_ACTION_WORD_END:
            *code16 = word_mods | KC_RIGHT;
            break;
        case VIM_ACTION_DOCUMENT_START:
            *code16    = document_start;
            next_vline = VLINE_UP;
            break;
        case VIM_ACTION_DOCUMENT_END:
            *code16    = document_end;
            next_vline = VLINE_DOWN;
            break;
        case VIM_ACTION_PAGE_UP:
            *code16         = KC_PAGE_UP;
            next_vline      = VLINE_UP;
            pending.keycode = KC_NO;
            break;
        case VIM_ACTION_PAGE_DOWN:
            *code16         = KC_PAGE_DOWN;
            next_vline      = VLINE_DOWN;
            pending.keycode = KC_NO;
            break;
        case VIM_ACTION_PASTE:
            *code16           = command_mods | KC_V;
            pending.keycode   = KC_NO;
            selection_cleared = true;
            break;
        case VIM_ACTION_UNDO:
            *code16         = command_mods | KC_Z;
            pending.keycode = KC_NO;
            break;
        case VIM_ACTION_JOIN_LINE:
            code16[0]         = line_end;
            code16[1]         = KC_SPACE;
            code16[2]         = KC_DEL;
            selection_cleared = true;
            break;
        default:
            break;
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
        *code16 |= QK_LSFT;
        type = VIM_SEND_TAP;
    }

    if (action & VIM_MOD_SELECT) {
        *code16 |= QK_LSFT;
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

    int8_t repeat = (pending.repeat == 0) ? 1 : pending.repeat;

    if ((action & VIM_MASK_ACTION) == VIM_ACTION_LINE) {
        type = VIM_SEND_TAP;
        vim_send(line_start, type);
        const uint16_t end_right[] = {LSFT(line_end), LSFT(KC_RIGHT)};
        vim_send_repeated_multi(repeat, end_right, 2);
        repeat = 1;
    }

    if (code16[2] != KC_NO) {
        vim_send_repeated_multi(repeat, code16, 3);
    } else if (code16[1] != KC_NO) {
        vim_send_repeated_multi(repeat, code16, 2);
    } else if (code16[0] != KC_NO) {
        // keycode is KC_NO in visual mode, where the object is the visual
        // selection
        vim_send_repeated(repeat, *code16, type);
    }

    if (vim_get_mode() == VIM_MODE_VLINE) {
        // select the full line after we release the up/down key, or after a tap
        if (type & VIM_SEND_RELEASE) {
            if (vline == VLINE_UP) {
                vim_send(LSFT(line_start), VIM_SEND_TAP);
            } else if (vline == VLINE_DOWN) {
                vim_send(LSFT(line_end), VIM_SEND_TAP);
            }
        }
    }

    if (action & VIM_MOD_DELETE) {
        vim_send(command_mods | KC_X, VIM_SEND_TAP);
        selection_cleared = true;
    } else if (action & VIM_MOD_YANK) {
        vim_send(command_mods | KC_C, VIM_SEND_TAP);
    }

    // Selecting the whole line leaves us on the beginning of the next one.
    // The line is still there after yanking, we need to return to it and also
    // get rid of the selection.
    if (action == (VIM_ACTION_LINE | VIM_MOD_YANK)) {
        vim_send(KC_LEFT, VIM_SEND_TAP);
        selection_cleared = true;
    }

    VIM_DPRINTF("vim_perform_action %x\n", action);
    vim_enter_mode(VIM_MODE_FROM_ACTION(action), selection_cleared);
}
