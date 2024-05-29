#pragma once
#include "quantum/logging/print.h"

#ifdef VIM_DEBUG
#    define VIM_DPRINT(s) dprint("[vim] " s)
#    define VIM_DPRINTF(...) dprintf("[vim] " __VA_ARGS__)
#else
#    define VIM_DPRINT(s) ((void)0)
#    define VIM_DPRINTF(...) ((void)0)
#endif

