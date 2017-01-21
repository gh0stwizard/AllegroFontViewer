# Allegro Font Viewer

Yet another basic Font Viewer based on Allegro 5.

## Controls

* `Esc`: exit from the program.
* `F12`: reload configuration files and apply new settings instantly.

### Directory listing mode

* `Up`, `Down`: navigation in directory listing mode.
* `PgUp`, `PgDn`: fast navigation in the directory listing mode.
* `Backspace`: open the parent directory.
* `Enter`: enter into the selected directory or try to load the selected font.
* `Space`: switch between directory listing mode & font view one.

vim-like conversion:

* `K` (Up), `J` (Down): navigation in directory listing mode.
* `LSHIFT` + `K` (PgUp), `LSHIFT` + `J` (PgDn): fast navigation in the directory listing mode.
* `F` (Enter): enter into the selected directory or try to load the selected font.
* `D` (Backspace): open the parent directory.

### Font view mode

* `Insert`: enter to Typing mode (see below).
* `Enter`, `Backspace`, `Space`: change mode to directory listing.
* `F2` - `F11`: load a text preset.
* Point a mouse to see a font size at bottom of the window.

vim-like conversion:

* `I` (Insert): enter to Typing mode (see below).
* `F` (Enter), `D` (Backspace): change mode to directory listing.

### Typing mode

* `F2` - `F11`: load a text preset.
* `Enter`: apply either a choosed preset or typed by hand text.
* `Backspace`: remove previous typed character.
* `Delete`: clear typed text.
* Anything else (mostly) will be typed as text.


## Dependecies

* `Allegro 5`: this version was written and tested under allegro-5.2.2 only.
* `ICU`: optional, used for file sorting. Use `-D_NO_ICU` to build without it.
