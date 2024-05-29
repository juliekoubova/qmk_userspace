#include "debug.h"
#include "pending.h"
#include "quantum/quantum.h"
#include "vim_mode.h"
#include "vim_send.h"

static vim_mode_t      vim_mode      = VIM_MODE_INSERT;
static vim_key_state_t vim_key_state = VIM_KEY_NONE;
static uint8_t         vim_mods      = 0; // we modify the actual mods so we can't rely on them

static void vim_cancel_os_selection(void) {
    vim_send(0, KC_LEFT, VIM_SEND_TAP);
}

vim_mode_t vim_get_mode(void) {
    return vim_mode;
}

__attribute__((weak)) void vim_mode_changed(vim_mode_t mode) {}

void vim_enter_insert_mode(void) {
    if (vim_mode == VIM_MODE_INSERT) {
        return;
    }
    VIM_DPRINT("Entering insert mode\n");
    vim_mode = VIM_MODE_INSERT;
    vim_mods = 0;
    vim_clear_pending();
    clear_keyboard_but_mods();
    vim_mode_changed(vim_mode);
}

void vim_enter_command_mode(void) {
    if (vim_mode == VIM_MODE_COMMAND) {
        return;
    }

    VIM_DPRINTF("Entering command mode vim_mods=%x\n", vim_mods);

    if (vim_mode == VIM_MODE_VISUAL) {
        vim_cancel_os_selection();
    }
    vim_mode = VIM_MODE_COMMAND;
    vim_mods = get_mods();
    vim_clear_pending();
    clear_keyboard_but_mods();
    vim_mode_changed(vim_mode);
}

void vim_enter_visual_mode(void) {
    if (vim_mode == VIM_MODE_VISUAL) {
        return;
    }
    VIM_DPRINT("Entering visual mode\n");
    vim_mode = VIM_MODE_VISUAL;

    // don't return to insert after vim key is released
    vim_set_vim_key_state(VIM_KEY_NONE);
    vim_clear_pending();

    clear_keyboard_but_mods();
    vim_mode_changed(vim_mode);
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
