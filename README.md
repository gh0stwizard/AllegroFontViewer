# Allegro Font Viewer

Yet another basic Font Viewer based on Allegro 5.

## Controls

* `Up`, `Down`: select either a directory or a file.
* `Enter`: enter into the selected directory or show file information.
* `Backspace`: open the parent directory.
* `L`: load selected font.
* `Esc`: exit from the program.
* `Space`: draw the directory listing (again)

## Dependecies

* `Allegro 5`: this version was written and tested under allegro-5.2.2 only.
* `ICU`: optional, used for file sorting. Use `-D_NO_ICU` to build without it.
