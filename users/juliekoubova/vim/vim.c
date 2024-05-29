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
#include "vim_mode.h"
#include "pending.h"
#include "perform_action.h"
#include "statemachine.h"
#include <stdbool.h>

void vim_process_command(uint16_t keycode, const keyrecord_t *record) {
    const vim_statemachine_t *state = vim_lookup_statemachine(keycode);
    vim_dprintf_state(state);
    if (!state) {
        return;
    }
    if (record->event.pressed) {
        if (state->append_if_pending) {
            if (vim_has_pending()) {
                vim_append_pending(keycode);
            } else if (state->action) {
                vim_perform_action(state->action, VIM_SEND_TAP);
            }
        } else if (state->append && state->action && vim_get_pending().keycode == keycode) {
            vim_perform_action(state->action, VIM_SEND_TAP);
        } else if (state->append) {
            vim_append_pending(keycode);
        } else if (state->repeating) {
            vim_perform_action(state->action, VIM_SEND_PRESS);
        } else {
            vim_perform_action(state->action, VIM_SEND_TAP);
        }
    } else if (state->repeating) {
        vim_perform_action(state->action, VIM_SEND_RELEASE);
    }
}

void vim_process_vim_key(bool pressed) {
    if (pressed) {
        if (vim_get_mode() == VIM_MODE_INSERT) {
            VIM_DPRINT("Vim key pressed in insert mode\n");
            vim_set_vim_key_state(VIM_KEY_TAP);
            vim_enter_command_mode();
        } else {
            VIM_DPRINT("Vim key pressed in non-insert mode\n");
            vim_set_vim_key_state(VIM_KEY_NONE);
            vim_enter_insert_mode();
        }
    } else {
        VIM_DPRINTF("Vim key released, vim_key_state=%d\n", vim_get_vim_key_state());
        switch (vim_set_vim_key_state(VIM_KEY_NONE)) {
            case VIM_KEY_NONE:
                // set when visual mode is entered from a vi key tap.
                // in that case, we want to stay in the selected mode
                break;
            case VIM_KEY_TAP:
                vim_enter_command_mode();
                break;
            case VIM_KEY_HELD:
                vim_enter_insert_mode();
                break;
        }
    }
}

bool vim_process_record_logged(uint16_t keycode, const keyrecord_t *record, uint16_t vim_keycode) {
    if (keycode == vim_keycode) {
        vim_process_vim_key(record->event.pressed);
        return false;
    }
    if (vim_get_mode() != VIM_MODE_INSERT) {
        if (record->event.pressed) {
            vim_set_vim_key_state(VIM_KEY_HELD);
        }
        if (IS_MODIFIER_KEYCODE(keycode)) {
            vim_set_mod(keycode, record->event.pressed);
            return false;
        }
        VIM_DPRINTF("vim_key_state=%d\n", vim_get_vim_key_state());
        vim_process_command(keycode, record);
        return false;
    }
    return true;
}

bool process_record_vim(uint16_t keycode, const keyrecord_t *record, uint16_t vim_keycode) {
    vim_dprintf_key("BEFORE", keycode, record);
    bool result = vim_process_record_logged(keycode, record, vim_keycode);
    vim_dprintf_key("AFTER", keycode, record);
    VIM_DPRINT("\n");
    return result;
}
