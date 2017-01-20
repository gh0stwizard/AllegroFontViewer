# Allegro Font Viewer

Yet another basic Font Viewer based on Allegro 5.

## Controls

* `Up`, `Down`: navigation in directory listing mode.
* `Enter`: enter into the selected directory or load the selected font.
* `Backspace`: open the parent directory.
* `Esc`: exit from the program.
* `Space`: switch between directory listing mode & font view one.
* `PgUp`, `PgDown`: fast navigation in the directory listing mode.

## Dependecies

* `Allegro 5`: this version was written and tested under allegro-5.2.2 only.
* `ICU`: optional, used for file sorting. Use `-D_NO_ICU` to build without it.
