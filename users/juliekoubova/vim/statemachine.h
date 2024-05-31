#pragma once
#include "debug.h"
#include "vim_mode.h"

#define VIM_MODE_ACTION(m) (m << 12)
#define VIM_MODE_FROM_ACTION(a) ((a >> 12) & 0xf)

typedef enum {
    VIM_ACTION_NONE = 0,

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

    VIM_MOD_DELETE = 0x0100,
    VIM_MOD_SELECT = 0x0200,
    VIM_MOD_YANK   = 0x0400,

    VIM_ENTER_INSERT  = VIM_MODE_ACTION(VIM_MODE_INSERT),
    VIM_ENTER_COMMAND = VIM_MODE_ACTION(VIM_MODE_COMMAND),
    VIM_ENTER_VISUAL  = VIM_MODE_ACTION(VIM_MODE_VISUAL),
    VIM_ENTER_VLINE  = VIM_MODE_ACTION(VIM_MODE_VLINE),

    VIM_MASK_ACTION = 0x00ff,
    VIM_MASK_MOD    = 0x0f00,
    VIM_MASK_MODE   = 0xf000
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
