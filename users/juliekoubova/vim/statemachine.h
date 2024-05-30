#pragma once
#include "debug.h"
#include "vim_mode.h"

typedef enum {
    VIM_ACTION_NONE,

    VIM_ACTION_LEFT,
    VIM_ACTION_DOWN,
    VIM_ACTION_UP,
    VIM_ACTION_RIGHT,

    VIM_ACTION_DOCUMENT_START,
    VIM_ACTION_DOCUMENT_END,
    VIM_ACTION_LINE_START,
    VIM_ACTION_LINE_END,
    VIM_ACTION_LINE,
    VIM_ACTION_PAGE_UP,
    VIM_ACTION_PAGE_DOWN,
    VIM_ACTION_WORD_START,
    VIM_ACTION_WORD_END,
    VIM_ACTION_SELECTION,

    VIM_ACTION_PASTE,
    VIM_ACTION_UNDO,
    VIM_ACTION_OPEN_LINE_UP,
    VIM_ACTION_OPEN_LINE_DOWN,
    VIM_ACTION_JOIN_LINE,

    VIM_MOD_MOVE          = 0,
    VIM_MOD_DELETE        = 0x0100,
    VIM_MOD_SELECT        = 0x0200,
    VIM_MOD_YANK          = 0x0400,

    VIM_MOD_INSERT_AFTER  = 0x1000,
    VIM_MOD_VISUAL_AFTER  = 0x2000,
    VIM_MOD_COMMAND_AFTER = 0x4000,

    VIM_MASK_ACTION = 0x00ff,
    VIM_MASK_MOD    = 0xff00,
} vim_action_t;

typedef struct {
    bool         append : 1;
    bool         append_if_pending : 1;
    bool         repeating : 1;
    vim_action_t action;
#ifdef VIM_DEBUG
    const char *debug;
#endif
} vim_statemachine_t;

const vim_statemachine_t *vim_lookup_statemachine(uint16_t keycode);

// Returns true for keys that are mapped in the current VIM mode.
// Useful for indicating the current mode using RGB matrix lights.
bool vim_is_active_key(uint16_t keycode);

void vim_dprintf_state(const vim_statemachine_t *state);
