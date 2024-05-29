ifeq ($(strip $(VIM_MODE_ENABLE)), yes)
  SRC += users/juliekoubova/vim/vim.c
  SRC += users/juliekoubova/vim/vim_mode.c
  SRC += users/juliekoubova/vim/vim_send.c
  SRC += users/juliekoubova/vim/pending.c
  SRC += users/juliekoubova/vim/perform_action.c
  SRC += users/juliekoubova/vim/statemachine.c
endif
