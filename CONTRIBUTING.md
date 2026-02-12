## Contributing

All contributions that furthers this project's **objective** are welcome.
As of right now, this project needs quantitative testing of generated QR codes.

How to contribute with **testing**:
1. Compile the code with `make debug`;
2. Generate _at least_ ten QR codes for each version and try to scan it with your mobile device, try edge cases, like switching subsets;
3. Open an issue in this repo with your results (even if all scans succeeded).

How to contribute with **code**:
1. Before opening any PR, create an issue discussing your proposed changes and why they are necessary (e.g. better design pattern);
2. Before commiting code, make sure to run it through [Valgrind](https://valgrind.org) and [Cppcheck](https://cppcheck.sourceforge.io);
3. Unless requiring change, use `clang` with `gnu11` standard (i.e. `c11` with GNU extensions).
4. Compile on Windows and Linux (must compile on `amd64`, others are optional) before pushing the code.

### Maintainer's notes

- Bitmap and SVG will be the only output formats supported. Monochrome bitmaps are packed (not compressed) into bits, but rows are padded to 32-bit longs. Still, it keeps the raster data quite small while avoiding compression. SVGs can be modified to change background/foreground color (and scaling beyond `-s` option compared to bitmaps).
- Higher Versions and EC levels are planned in the near future. Those require (a) codewords to be woven; and (b) additional version information to be placed on code.
- Kanji encoding is not likely to be supported in the future, even though it is part of the ISO standard.
