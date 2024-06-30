Hey everyone, this is my [QMK userspace](https://docs.qmk.fm/newbs_external_userspace).

# Vim Mode

Perchance you're interested in my Vim mode. It is pretty comprehensive, if I dare
say so myself. I have built it for my 60% keyboard, and also to reduce the
differences switching between Windows, Linux, and macOS all day long.

It uses a single `QK_VIM` keycode&mdash;I have this mapped on my `Caps Lock` key.

You can either tap the `QK_VIM` key to ~~enter command mode~~ leave insert mode
(like `Esc` in actual Vim), or you can hold it like a modifier, and the command
mode will disengage once you release it. This is useful for simple navigation,
e.g. when your keyboard is missing dedicated arrow keys, or you just don't like
leaving your home row.

If you enter Vim command mode, exiting is very easy, compared to the real thing.
You just press that key again.

### Motions
* Replace your arrows: `h`, `j`, `k`, `l`
    * you still want real arrow keys in some layer, if you're sporting 60% or
      better. Preferably on your left hand.
    * `QK_VIM`+`h`/`j`/`k`/`l` works great, but you can't combine it modifier
      keys.
* Jump over words: `W`, `w`, `B`, `b`
    * sends `Ctrl`+`←`/`→` or `Option`+`←`/`→` in Apple mode
    * lower and upper case obviously do the same thing
* Motions can be repeated (e.g. `5j` goes five lines down)
    * ⚠️ this should be considered experimental, I plan to reimplement this to
      make it possible to interrupt by pressing a key
    * if it goes haywire, you have to unplug the keyboard for now
* Line begin and end: `0`, `^`, `$`
    * sends `Home`/`End` or `Cmd`+`←`/`→` on Mac
    * `0` and `^` do the same thing again
* Document begin and end (`gg` and `G`)
    * sends `Ctrl`+`Home`/`End` or `Cmd`+`↑`/`↓` on Mac
* Page Up / page down (`Ctrl`+`B`, `Ctrl`+`F`)
    * sends `PageUp`, `PageDown`

### Commands
* `c`, `d` and `y` do what you would expect. You can repeat them (e.g. `5dw`)
* `cc`, `dd`, `S`, and `yy` do what you would expect, at least most of the time.
    * ⚠️ They don't play well with soft-wrapped lines.
* `J`, `o`, and `O` also work
* `p` and `P` work, but they do the same thing (`Ctrl`/`Cmd`+`V`)
* `u` sends `Ctrl`/`Cmd`+`Z`

### Visual and V-Line Modes
* `v` puts you in visual mode
* `V` puts you in v-line mode
    * ⚠️ it kinda breaks if you start going one direction, then reverse and go
      past the original starting point

## Setup Instructions
To try it out, I suggest adding my userspace as a git submodule and linking it
into your `users` folder. This should work the same whether you use
[external userspace](https://docs.qmk.fm/newbs_external_userspace), 
or you have forked [the main repo](https://github.com/qmk/qmk_firmware/).

```shell
$ mkdir submodules
$ git submodule add https://github.com/juliekoubova/qmk_userspace.git submodules/juliekoubova
$ ln -s submodules/juliekoubova/users/juliekoubova users/juliekoubova
```

Now you should be able to include and enable Vim mode in your keymap's `rules.mk`:

```make
VIM_MODE_ENABLE = yes
include users/juliekoubova/rules.mk
```

All that remains is to define a `QK_VIM` key, include it in your keymap, and
call `process_record_vim` from your `process_record_user`. For a more advanced
example, including RGB gamer vomit on mapped keys, check out 
[my Keychron Q4 keymap](https://github.com/juliekoubova/qmk_userspace/blob/main/keyboards/keychron/q4/ansi/keymaps/juliekoubova/keymap.c).

```c
// 1. include the header
#include "users/juliekoubova/vim.h"

// 2. define a QK_VIM code
enum key_codes {
    QK_VIM = SAFE_RANGE,
};

// 3. use it in your layout

// 4. call process_record_vim
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return process_record_vim(keycode, record, QK_VIM);
}
```

### macOS Support
You can call `vim_set_apple(true)` to tell Vim mode to send macOS shortcuts.
This pairs nicely with QMK's built-in [OS detection](https://docs.qmk.fm/features/os_detection):
```c
#ifdef OS_DETECTION_ENABLE
bool process_detected_host_os_user(os_variant_t os) {
    vim_set_apple(os == OS_MACOS || os == OS_IOS);
    return true;
}
#endif
```

## Roadmap
* I need to make repeats asynchronous, so they can be cancelled without
  rebooting the keyboard
* Once that is done, macros: `q` and `@`
* Maybe `:bn` and `:bp` for `Ctrl`(+`Shift`)+`Tab` vs. `Cmd`+`{`/`}` on Mac
* what else?
