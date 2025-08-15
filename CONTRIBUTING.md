## Contributing

All contributions that furthers this project's **objective** are welcome.
As of right now, this project needs quantitative testing of generated QR codes.

How to contribute with **testing**:
1. Compile the code with `make` (use MinGW or other on Windows);
1. Generate _at least_ two QR codes for each version and try to scan it with your mobile device;
1. Open an issue in this repo with your results (even if all scans succeeded).

How to contribute with **code**:
1. Before opening any PR, create an issue discussing your proposed changes and why they are necessary (e.g. better design pattern);
1. Compile on Windows and Linux before pushing the code;
1. If your code does anything with heap memory, make sure to run it through [Valgrind](https://valgrind.org).

### Maintainer's notes

- Bitmap and SVG will be the only output formats supported. Monochrome bitmaps are packed (not compressed) into bits, but rows are padded to 32-bit longs, still it keeps the raster data quite small while avoiding compression. SVGs can be modified to change background/foreground color (and scaling beyond `-s` option for bitmaps).
- Higher Versions and EC levels are planned in the near future. Those require (a) codewords to be weaved; and (b) additional version information to be placed on code.
- Kanji encoding is not likely to be supported in the future, even though it is part of the ISO standard.
- Roadmap:
    - [x] v1.4.0 -- vector output
    - [ ] v1.5.0 -- encoding swithing
    - [ ] v1.6.0 -- higher EC levels
    - [ ] v1.7+  -- higher Version codes
