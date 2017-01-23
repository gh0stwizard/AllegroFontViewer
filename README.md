# Allegro Font Viewer

Yet another basic Font Viewer based on Allegro 5.

## Controls

* `Esc`: exit from the program.
* `F1`: show the help.
* `F12`: reload configuration files and apply new settings instantly.

### Directory listing mode

* `Up`, `Down`: navigation in directory listing mode.
* `PgUp`, `PgDn`: quick navigation in the directory listing mode.
* `Backspace`: open a parent directory.
* `Enter`: enter into the selected directory or try to load the selected font.
* `Space`: switch between directory listing mode & font view one.

vim-like keys conversion:

* `K` (Up), `J` (Down): navigation in the directory listing mode.
* `LSHIFT` + `K` (PgUp), `LSHIFT` + `J` (PgDn): quick navigation in the directory listing mode.
* `F` (Enter): enter into the selected directory or try to load the selected font.
* `D` (Backspace): open a parent directory.

### Font view mode

* `Insert`: switch to the typing mode, see below.
* `Enter`, `Backspace`, `Space`: change the mode to the directory listing one.
* `F2` - `F11`: load a text preset.
* Point a mouse to see a font size at bottom of the window.

vim-like keys conversion:

* `I` (Insert): switch to the typing mode, see below.
* `F` (Enter), `D` (Backspace): change the mode to the directory listing one.

### Typing mode

* `F2` - `F11`: load a text preset.
* `Enter`: apply a typed text.
* `Backspace`: remove a previously typed character.
* `Delete`: clear whole typed text.
* Other keys are considered as a characters for the typing text.


## Dependecies

* `Allegro 5`: this version was written and tested under allegro-5.2.2 only.
* `ICU`: optional, used for file sorting. Use `-D_NO_ICU` to build without it.
