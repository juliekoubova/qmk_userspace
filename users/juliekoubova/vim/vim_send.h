#pragma once
#include <stdint.h>

typedef enum {
    VIM_SEND_TAP,
    VIM_SEND_PRESS,
    VIM_SEND_RELEASE,
} vim_send_type_t;

void vim_send(uint8_t mods, uint16_t keycode, vim_send_type_t type);

