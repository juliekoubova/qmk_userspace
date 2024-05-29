#include "vim_send.h"
#include "debug.h"
#include "quantum/quantum.h"

#define VIM_TAP_DELAY 30

void vim_send(uint8_t mods, uint16_t keycode, vim_send_type_t type) {
    if (mods && (type == VIM_SEND_PRESS || type == VIM_SEND_TAP)) {
        VIM_DPRINTF("register mods=%x\n", mods);
        register_mods(mods);
    }

    switch (type) {
        case VIM_SEND_TAP:
            VIM_DPRINTF("tap keycode=%x\n", keycode);
            tap_code_delay(keycode, VIM_TAP_DELAY);
            break;
        case VIM_SEND_PRESS:
            VIM_DPRINTF("register keycode=%x\n", keycode);
            register_code(keycode);
            wait_ms(VIM_TAP_DELAY);
            break;
        case VIM_SEND_RELEASE:
            VIM_DPRINTF("unregister keycode=%x\n", keycode);
            unregister_code(keycode);
            wait_ms(VIM_TAP_DELAY);
            break;
    }

    if (mods && (type == VIM_SEND_RELEASE || type == VIM_SEND_TAP)) {
        VIM_DPRINTF("unregister mods=%x\n", mods);
        unregister_mods(mods);
    }
}

