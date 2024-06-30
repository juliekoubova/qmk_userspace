# Julie's QMK Userspace

Hey everyone, this is my QMK userspace.

# Vim Mode

Perchance you're interested in my Vim mode. It is pretty comprehensive, if I dare say so myself. I have built it
for my 60% keyboard, and also to reduce the differences switching between Windows, Linux, and macOS all day long.

It uses a single `QK_VIM` keycode&mdash;I have this mapped on my `Caps Lock` key.

You can either tap the `QK_VIM` key to enter command mode (like `Esc` in actual Vim), or you can hold like a modifier,
and the command mode will disengage once you release it. This is useful for simple navigation, e.g. when your keyboard
is missing dedicated arrow keys, or you just don't like leaving your home row.

If you enter Vim command mode, exiting is very easy, compared to the real thing. You just press that key again.

## Motions
* Replace your arrows: `h`, `j`, `k`, `l`
    * you still want real arrow keys in some layer, if you're sporting 60% or better. Preferably on your left hand.
    * `Vim`+`hjkl` works great, but you can't combine it modifier keys.
* Jump over words: `W`, `w`, `B`, `b`
    * sends `Ctrl`+`←`/`→` or `Option`+`←`/`→` in Apple mode
    * lower and upper case obviously do the same thing, can't do any better in a keyboard
* Motions can be repeated (e.g. `5j` goes five lines down)
    * ⚠️ this should be considered experimental, I plan to reimplement this to make it possible to interrupt by pressing a key,
      in case it goes haywire
    * if it does go haywire, you'll have to unplug the keyboard for now
* Line begin and end: `0`, `^`, `$`
    * sends `Home`/`End` or `Cmd`+`←`/`→` on Mac
    * `0` and `^` do the same thing again
* Document begin and end (`gg` and `G`)
    * sends `Ctrl`+`Home`/`End` or `Cmd`+`↑`/`↓` on Mac
* Page Up / page down (`Ctrl`+`B`, `Ctrl`+`F`)
    * sends `PageUp`, `PageDown`
 
## Commands
* `c`, `d` and `y` do what you would expect. You can repeat them (e.g. `5dw`)
* `cc`, `dd`, `J`, `o`, `O`, `S`, and `yy` do what you would expect, at least most of the time.
    * ⚠️ They don't play well with soft-wrapped lines.
* `p` and `P` work, but they do the same thing (`Ctrl`/`Cmd`+`V`)
* `u` sends `Ctrl`/`Cmd`+`Z`

## Visual and V-Line Modes
* `v` and `V` put you in visual mode
    * ⚠️ V-LINE mode kinda breaks if you start going one direction, then reverse and go past  the original starting point

## Setup Instructions
To try it out, I suggest adding my userspace as a git submodule and linking it into your `users` folder. 
This should work the same whether you also have [an external userspace](https://github.com/qmk/qmk_userspace/), 
or you have forked [qmk_firmware](https://github.com/qmk/qmk_firmware/).

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

All that remains is to define a `QK_VIM` key, include it in your keymap, and call `process_record_vim`
from your `process_record_user`. For more advanced example, check out 
[my Keychron Q4 keymap](https://github.com/juliekoubova/qmk_userspace/blob/main/keyboards/keychron/q4/ansi/keymaps/juliekoubova/keymap.c).


```c
// 1. define a QK_VIM code
enum key_codes {
    QK_VIM = SAFE_RANGE,
};

// 2. use it in your layout

// 3. call process_record_vim
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return process_record_vim(keycode, record, QK_VIM);
}
```
### macOS Support
You can call `vim_set_apple(true)` to switch Vim mode to send macOS shortcuts. This pairs nicely with QMK's
builtin OS detection:
```c
#ifdef OS_DETECTION_ENABLE
bool process_detected_host_os_user(os_variant_t os) {
    vim_set_apple(os == OS_MACOS || os == OS_IOS);
    return true;
}
#endif
```
