# Melonbar
I need a status bar for my laptop, so I made this.

## Dependencies
- `libX11`
- `freetype2`

## Building
1. Run `$ make` to build the melonbar binary
2. Run `$ make config` to copy the config file to your `~/.config/` directory
3. Run `# make install` to install the binary to `/usr/local/bin`. You can change the install path through the `INSTALL_PATH` variable

## TO-DO 
- Fix command line flags
- Optimize the shitshow that is `include/config.c`... and honestly everything else
- Add some kind of default settings
- Possibly add clickable modules?

## License
Licensed under the WTFPL. Do whatever you please.
