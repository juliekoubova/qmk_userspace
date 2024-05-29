#include "pending.h"
#include "debug.h"
#include "quantum/quantum.h"
#include <stdint.h>

#ifndef VIM_PENDING_MAX_REPEAT
#    define VIM_PENDING_MAX_REPEAT 10
#endif

static vim_pending_t vim_pending = {KC_NO, 0};

static void vim_dprintf_pending(void) {
    VIM_DPRINTF("pending repeat=%d keycode=%x\n", vim_pending.repeat, vim_pending.keycode);
}

void vim_append_pending(uint8_t keycode) {
    if (keycode == KC_0) {
        if (vim_pending.repeat > 0) {
            vim_pending.repeat *= 10;
        }
    } else if (keycode >= KC_1 && keycode <= KC_9) {
        vim_pending.repeat *= 10;
        vim_pending.repeat += 1 + (keycode - KC_1);
    } else {
        vim_pending.keycode = keycode;
    }
    vim_dprintf_pending();
}

vim_pending_t vim_clear_pending(void) {
    VIM_DPRINT("vim_clear_pending\n");
    vim_dprintf_pending();
    vim_pending_t previous = vim_pending;
    vim_pending.repeat     = 0;
    vim_pending.keycode    = KC_NO;
    return previous;
}

vim_pending_t vim_get_pending(void) {
    return vim_pending;
}

bool vim_has_pending(void) {
    return vim_pending.repeat > 0 || vim_pending.keycode != KC_NO;
}
