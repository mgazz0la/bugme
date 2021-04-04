# bugme

The bugme emulator is a personal project focused on learning how to write clean C++20 code, as well
as on a personal passion for low-level retro hardware emulation. Just to start, this project is
focused on DMG Gameboy feature (almost-)parity before exploring any SGB/GBC/etc emulation, although
that is the intended direction for this project.

## Build

Requires `cmake` and `SDL2`. This has only been tested on Ubuntu. It suffices to just clone and run
`make`.

## Run

`./build/bin/bugme`

```sh
usage: bugme <rom_file> [--debug] [--verbosity v] [--headless]

arguments:
  --debug                   Enable the debugger
  --verbosity               Specify a verbosity level (0-4)
  --headless                Run without a display (console output only)
```

## Further documentation

If you have `doxygen` installed, you may run it to generate an HTML class reference. Point your
browser at `html/index.html` to view.

## Missing Functionality

* Sprite support
* Joypad
* Debugger
* Sound

## Helpful links/references

* [Pandocs](http://bgb.bircd.org/pandocs.htm)
* [Sameboy](https://github.com/LIJI32/SameBoy)
* [jgilchrist/gbemu](https://github.com/jgilchrist/gbemu)
* [Blargg's tests](https://gbdev.gg8.se/wiki/articles/Test_ROMs)
* [Imran Nazar's gb.js](https://imrannazar.com/GameBoy-Emulation-in-JavaScript:-The-CPU)
