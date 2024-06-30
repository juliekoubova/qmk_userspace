ifeq ($(strip $(VIM_MODE_ENABLE)), yes)
  SRC += vim/pending.c
  SRC += vim/perform_action.c
  SRC += vim/statemachine.c
  SRC += vim/vim.c
  SRC += vim/vim_mode.c
  SRC += vim/vim_send.c
endif
