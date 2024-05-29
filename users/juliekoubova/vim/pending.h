#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "statemachine.h"

typedef struct {
    uint8_t keycode;
    uint8_t repeat;
} vim_pending_t;

void vim_append_pending(uint8_t keycode);
bool vim_has_pending(void);

vim_pending_t vim_clear_pending(void);
vim_pending_t vim_get_pending(void);
