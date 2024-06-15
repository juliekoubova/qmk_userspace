/* Copyright 2022 @ Keychron (https://www.keychron.com)
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

#include QMK_KEYBOARD_H
#include "vim.h"
#include <stdbool.h>

enum layers {
    BASE,
    FN,
};

enum key_codes {
    QK_VIM = SAFE_RANGE,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // clang-format off
    [BASE] = LAYOUT_ansi_61(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,      KC_0,    KC_MINS, KC_EQL,  KC_BSPC,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,      KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,
        QK_VIM,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,      KC_SCLN, KC_QUOT,          KC_ENT,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,    KC_SLSH,                   KC_RSFT,
        KC_LCTL, KC_LOPT, KC_LCMD,                            KC_SPC,                               MO(FN),  KC_RCMD, KC_ROPT, KC_RCTL),

    [FN] = LAYOUT_ansi_61(
        KC_GRAVE, KC_F1,   KC_F2,   KC_F3,    KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,    KC_F10,  KC_F11,  KC_F12,  KC_DEL,
        QK_BOOT,  _______, KC_UP,   _______,  _______, _______, _______, _______, _______, _______,  _______, KC_VOLD, KC_VOLU, KC_MUTE,
        _______,  KC_LEFT, KC_DOWN, KC_RIGHT, _______, _______, KC_LEFT, KC_DOWN, KC_UP,   KC_RIGHT, _______, _______,          _______,
        _______,  _______, _______, _______,  _______, _______, _______, _______, _______, _______,  _______,                   _______,
        _______,  _______, _______,                             _______,                             _______, _______, _______, _______)
    // clang-format on
};

#ifdef OS_DETECTION_ENABLE

static os_variant_t detected_os;

bool process_detected_host_os_user(os_variant_t os) {
    detected_os = os;
    dprintf("OS detected: %d\n", detected_os);
    vim_set_apple(os == OS_MACOS || os == OS_IOS);
    return true;
}

#endif

// ============================================================================
// RGB Matrix
// ============================================================================

#ifdef RGB_MATRIX_ENABLE

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t layer = get_highest_layer(layer_state);
    bool is_apple = detected_os == OS_MACOS || detected_os == OS_IOS;

    for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
            uint8_t index = g_led_config.matrix_co[row][col];
            if (index < led_min || index >= led_max || index == NO_LED) {
                continue;
            }

            if (get_highest_layer(layer_state) > 0) {
                uint16_t keycode = keymap_key_to_keycode(layer, (keypos_t){col, row});
                if (keycode > KC_TRNS) {
                    if (is_apple) {
                        rgb_matrix_set_color(index, RGB_RED);
                    } else {
                        rgb_matrix_set_color(index, RGB_GREEN);
                    }
                    continue;
                }
            };

            uint16_t keycode = keymap_key_to_keycode(BASE, (keypos_t){col, row});
            if (keycode == KC_LSFT) {
                if (is_caps_word_on() || host_keyboard_led_state().caps_lock) {
                    rgb_matrix_set_color(index, RGB_TURQUOISE);
                    continue;
                }
            } else if (vim_is_active_key(keycode)) {
                switch (vim_get_mode()) {
                    case VIM_MODE_VISUAL:
                        rgb_matrix_set_color(index, RGB_YELLOW);
                        break;
                    case VIM_MODE_VLINE:
                        rgb_matrix_set_color(index, RGB_SPRINGGREEN);
                        break;
                    default:
                        rgb_matrix_set_color(index, RGB_BLUE);
                        break;
                }
                continue;
            }

            rgb_matrix_set_color(index, RGB_OFF);
        }
    }
    return false;
}

#endif

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return process_record_vim(keycode, record, QK_VIM);
}

void housekeeping_task_user() {
    //vim_task();
}

#ifdef VIM_DEBUG
void keyboard_post_init_user(void) {
    debug_enable = true;
}
#endif

