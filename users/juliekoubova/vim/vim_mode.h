#pragma once
#include <stdint.h>
#include "quantum/quantum.h"

typedef enum {
    VIM_MODE_INSERT = 1,
    VIM_MODE_COMMAND,
    VIM_MODE_VISUAL,
    VIM_MODE_VLINE,
} vim_mode_t;

typedef enum { VIM_KEY_NONE, VIM_KEY_TAP, VIM_KEY_HELD } vim_key_state_t;

void    vim_set_mod(uint16_t keycode, bool pressed);
uint8_t vim_get_mods(void);

vim_key_state_t vim_get_vim_key_state(void);
vim_key_state_t vim_set_vim_key_state(vim_key_state_t);

void       vim_enter_command_mode(void);
void       vim_enter_insert_mode(void);
void       vim_enter_visual_mode(void);
void       vim_enter_vline_mode(void);
void       vim_enter_mode(vim_mode_t mode);
vim_mode_t vim_get_mode(void);
void       vim_mode_changed(vim_mode_t mode);

void vim_dprintf_key(const char *prefix, uint16_t keycode, const keyrecord_t *record);
