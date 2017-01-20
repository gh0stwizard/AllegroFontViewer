# Allegro Font Viewer

Yet another basic Font Viewer based on Allegro 5.

## Controls

* `Esc`: exit from the program.

### Directory listing mode

* `Up`, `Down`: navigation in directory listing mode.
* `Enter`: enter into the selected directory or try to load the selected font.
* `Backspace`: open the parent directory.
* `Space`: switch between directory listing mode & font view one.
* `PgUp`, `PgDown`: fast navigation in the directory listing mode.

### Font view mode

* `Insert`: enter to Typing mode (see below).
* `Enter`: change mode to directory listing.

### Typing mode

* `F2` - `F11`: load a text preset.
* `Enter`: apply either a choosed preset or typed by hand text.
* `Backspace`: remove previous typed character.
* `Delete`: clear typed text.
* Anything else (mostly) will be typed as text.


## Dependecies

* `Allegro 5`: this version was written and tested under allegro-5.2.2 only.
* `ICU`: optional, used for file sorting. Use `-D_NO_ICU` to build without it.
