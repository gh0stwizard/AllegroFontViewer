# Allegro Font Viewer

Yet another basic Font Viewer based on Allegro 5.

## Controls

* `Up`, `Down`: select either a directory or a file.
* `Enter`: enter into the selected directory or show file information.
* `Backspace`: open the parent directory.
* `L`: try to load selected file as a font and show it.
* `Esc`: exit from the program.
* `Space`: draw the directory listing (again)
* `PgUp`, `PgDown`: fast navigation in the directory listing mode.

## Dependecies

* `Allegro 5`: this version was written and tested under allegro-5.2.2 only.
* `ICU`: optional, used for file sorting. Use `-D_NO_ICU` to build without it.
