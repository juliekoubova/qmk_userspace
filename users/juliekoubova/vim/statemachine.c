#include "debug.h"
#include "statemachine.h"
#include "vim_mode.h"

#define VSM_FIRST KC_A
#define VSM_LAST KC_ESC
#define VSM_SIZE (VSM_LAST - VSM_FIRST + 1)
#define VSM_INDEX(key) ((key) >= VSM_FIRST && (key) <= VSM_LAST ? (key - VSM_FIRST) : -1)

#ifdef VIM_DEBUG
#    define VSM_DEBUG(str) .debug = str
#else
#    define VSM_DEBUG(str)
#endif

// clang-format off
#define VSM(key, a) [VSM_INDEX(key)] = { \
    .action = (a), \
    VSM_DEBUG(#key " => " #a) \
}
#define VSM_REPEATING(key, a) [VSM_INDEX(key)] = { \
    .action = (a), \
    .repeating = true, \
    VSM_DEBUG(#key " => " #a " (repeating)") \
}
#define VSM_APPEND(key) [VSM_INDEX(key)] = { \
    .append = true, \
    VSM_DEBUG(#key " => append to pending") \
}
#define VSM_APPEND_FIRST_THEN_ACTION(key, a) [VSM_INDEX(key)] = { \
    .action = (a), \
    .append = true, \
    VSM_DEBUG(#key " => " #a " (append to pending)") \
}
#define VSM_APPEND_IF_PENDING(key, a) [VSM_INDEX(key)] = { \
    .action = (a), \
    .append_if_pending = true, \
    VSM_DEBUG(#key " => " #a " (append if already pending)") \
}
// clang-format on

static const vim_statemachine_t vim_statemachine_command[VSM_SIZE] = {
    // clang-format off
    VSM(KC_A, VIM_ACTION_RIGHT | VIM_MOD_INSERT_AFTER),
    VSM_REPEATING(KC_B, VIM_ACTION_WORD_START),
    VSM_APPEND_FIRST_THEN_ACTION(KC_C, VIM_ACTION_LINE | VIM_MOD_CHANGE),
    VSM_APPEND_FIRST_THEN_ACTION(KC_D, VIM_ACTION_LINE | VIM_MOD_DELETE),
    VSM_REPEATING(KC_E, VIM_ACTION_WORD_END),
    VSM_APPEND_FIRST_THEN_ACTION(KC_G, VIM_ACTION_DOCUMENT_START),
    VSM_REPEATING(KC_H, VIM_ACTION_LEFT),
    VSM(KC_I, VIM_MOD_INSERT_AFTER),
    VSM_REPEATING(KC_J, VIM_ACTION_DOWN),
    VSM_REPEATING(KC_K, VIM_ACTION_UP),
    VSM_REPEATING(KC_L, VIM_ACTION_RIGHT),
    VSM_REPEATING(KC_P, VIM_ACTION_PASTE),
    VSM(KC_S, VIM_ACTION_RIGHT | VIM_MOD_CHANGE),
    VSM_REPEATING(KC_U, VIM_ACTION_UNDO),
    VSM(KC_V, VIM_ACTION_VISUAL_MODE),
    VSM_REPEATING(KC_W, VIM_ACTION_WORD_END),
    VSM_REPEATING(KC_X, VIM_ACTION_RIGHT | VIM_MOD_DELETE),
    VSM_APPEND_FIRST_THEN_ACTION(KC_Y, VIM_ACTION_LINE | VIM_MOD_YANK),
    VSM_APPEND(KC_1),
    VSM_APPEND(KC_2),
    VSM_APPEND(KC_3),
    VSM_APPEND(KC_4),
    VSM_APPEND(KC_5),
    VSM_APPEND(KC_6),
    VSM_APPEND(KC_7),
    VSM_APPEND(KC_9),
    VSM_APPEND(KC_9),
    VSM_APPEND_IF_PENDING(KC_0, VIM_ACTION_LINE_START),
    // clang-format on
};

static const vim_statemachine_t vim_statemachine_command_shift[VSM_SIZE] = {
    // clang-format off
    VSM(KC_A, VIM_ACTION_LINE_END | VIM_MOD_INSERT_AFTER),
    VSM_REPEATING(KC_B, VIM_ACTION_WORD_START),
    VSM(KC_C, VIM_ACTION_LINE_END | VIM_MOD_CHANGE),
    VSM(KC_D, VIM_ACTION_LINE_END | VIM_MOD_DELETE),
    VSM_REPEATING(KC_E, VIM_ACTION_WORD_END),
    VSM_REPEATING(KC_G, VIM_ACTION_DOCUMENT_END),
    VSM(KC_I, VIM_ACTION_LINE_START | VIM_MOD_INSERT_AFTER),
    VSM_REPEATING(KC_P, VIM_ACTION_PASTE),
    VSM(KC_S, VIM_ACTION_LINE | VIM_MOD_CHANGE),
    VSM(KC_V, VIM_ACTION_LINE | VIM_MOD_SELECT | VIM_MOD_VISUAL_AFTER),
    VSM_REPEATING(KC_W, VIM_ACTION_WORD_END),
    VSM_REPEATING(KC_X, VIM_ACTION_LEFT | VIM_MOD_DELETE),
    VSM(KC_Y, VIM_ACTION_LINE | VIM_MOD_YANK),
    VSM_REPEATING(KC_4, VIM_ACTION_LINE_END),
    VSM_REPEATING(KC_6, VIM_ACTION_LINE_START),
    // clang-format on
};

static const vim_statemachine_t vim_statemachine_command_ctrl[VSM_SIZE] = {
    VSM_REPEATING(KC_B, VIM_ACTION_PAGE_UP),
    VSM_REPEATING(KC_F, VIM_ACTION_PAGE_DOWN),
};

static const vim_statemachine_t vim_statemachine_visual[VSM_SIZE] = {
    // clang-format off
    VSM_REPEATING(KC_B, VIM_ACTION_WORD_START | VIM_MOD_SELECT),
    VSM(KC_C, VIM_ACTION_SELECTION | VIM_MOD_CHANGE),
    VSM(KC_D, VIM_ACTION_SELECTION | VIM_MOD_DELETE | VIM_MOD_COMMAND_AFTER),
    VSM_REPEATING(KC_E, VIM_ACTION_WORD_END | VIM_MOD_SELECT),
    VSM_APPEND_FIRST_THEN_ACTION(KC_G, VIM_ACTION_DOCUMENT_START | VIM_MOD_SELECT),
    VSM_REPEATING(KC_H, VIM_ACTION_LEFT | VIM_MOD_SELECT),
    VSM_REPEATING(KC_J, VIM_ACTION_DOWN | VIM_MOD_SELECT),
    VSM_REPEATING(KC_K, VIM_ACTION_UP | VIM_MOD_SELECT),
    VSM_REPEATING(KC_L, VIM_ACTION_RIGHT | VIM_MOD_SELECT),
    VSM_REPEATING(KC_P, VIM_ACTION_PASTE),
    VSM(KC_S, VIM_ACTION_SELECTION | VIM_MOD_CHANGE),
    VSM(KC_V, VIM_ACTION_COMMAND_MODE),
    VSM_REPEATING(KC_W, VIM_ACTION_WORD_END | VIM_MOD_SELECT),
    VSM(KC_X, VIM_ACTION_SELECTION | VIM_MOD_DELETE),
    VSM(KC_Y, VIM_ACTION_SELECTION | VIM_MOD_YANK | VIM_MOD_COMMAND_AFTER),
    VSM_APPEND(KC_1),
    VSM_APPEND(KC_2),
    VSM_APPEND(KC_3),
    VSM_APPEND(KC_4),
    VSM_APPEND(KC_5),
    VSM_APPEND(KC_6),
    VSM_APPEND(KC_7),
    VSM_APPEND(KC_9),
    VSM_APPEND(KC_9),
    VSM_APPEND_IF_PENDING(KC_0, VIM_ACTION_LINE_START | VIM_MOD_SELECT),
    VSM(KC_ESCAPE, VIM_ACTION_COMMAND_MODE),
    // clang-format on
};

static const vim_statemachine_t vim_statemachine_visual_shift[VSM_SIZE] = {
    // clang-format off
    VSM(KC_C, VIM_ACTION_LINE | VIM_MOD_CHANGE),
    VSM(KC_D, VIM_ACTION_LINE | VIM_MOD_DELETE),
    VSM(KC_V, VIM_ACTION_LINE | VIM_MOD_SELECT),
    VSM(KC_X, VIM_ACTION_LINE | VIM_MOD_DELETE),
    VSM(KC_Y, VIM_ACTION_LINE | VIM_MOD_YANK),
    VSM(KC_4, VIM_ACTION_LINE_END | VIM_MOD_SELECT),
    VSM(KC_6, VIM_ACTION_LINE_START | VIM_MOD_SELECT),
    // clang-format on
};

#undef VSM
#undef VSM_DEBUG
#undef VSM_REPEATING
#undef VSM_APPEND
#undef VSM_APPEND_IF_PENDING
#undef VSM_APPEND_THEN_ACTION

const vim_statemachine_t *vim_lookup_statemachine(uint16_t keycode) {
    if (keycode < VSM_FIRST || keycode > VSM_LAST) {
        return NULL;
    }

    uint8_t vim_mods = vim_get_mods();
    uint8_t index    = VSM_INDEX(keycode);
    bool    ctrl     = (vim_mods & MOD_BIT(KC_LCTL)) || (vim_mods & MOD_BIT(KC_RCTL));
    bool    shift    = (vim_mods & MOD_BIT(KC_LSFT)) || (vim_mods & MOD_BIT(KC_RSFT));

    switch (vim_get_mode()) {
        case VIM_MODE_COMMAND:
            if (ctrl) {
                return &vim_statemachine_command_ctrl[index];
            } else if (shift) {
                return &vim_statemachine_command_shift[index];
            } else if (vim_mods == 0) {
                return &vim_statemachine_command[index];
            }
            break;
        case VIM_MODE_VISUAL:
            if (vim_mods == 0) {
                return &vim_statemachine_visual[index];
            } else if (shift) {
                return &vim_statemachine_visual_shift[index];
            }
            break;

        default:
            break;
    }

    return NULL;
}

bool vim_is_active_key(uint16_t keycode) {
    if (vim_get_mode() == VIM_MODE_INSERT) {
        return false;
    }

    const vim_statemachine_t *state = vim_lookup_statemachine(keycode);
    return state != NULL && state->action != VIM_ACTION_NONE;
}

void vim_dprintf_state(const vim_statemachine_t *state) {
    VIM_DPRINTF("state=%s\n", state ? state->debug : "NULL");
}
