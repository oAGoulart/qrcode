# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

- Refactor codebase with `clang-tidy`

## [1.11.0] - 2025-09-05

Add subset mode switching and data bits packing

- Add `--optimize` option to reduce data size
- Add numeric and alphanumeric encoding in addition to byte
- Add platform and architecture to `--version` option output
- Add support for FreeBSD, Solaris and macOS
- Add `heaparray` and `packedbits` interfaces
- Change error messages to comply with GNU style guide
- Change `-G` option name to `-K`
- Change compiler to `clang`
- Change lookup table generation to build time
- Change standard calls with `clang`'s builtins
- Change encoding stream to `packedbits` interface
- Change language standard to ISO C with GNU extensions
- Remove `array_pop` from byte encoding, optimize xoring arrays

## [1.4.0] - 2025-08-15

Add SVG output option

- Added scalable vector graphics output option `-G`, disregards scaling option
- Added version information option `--version`
- Changed option `-v` name to `-u`
- Fixed memory leak on fatal exit

## [1.3.0] - 2025-08-11

Add bitmap output option

- Added option `-B` to write generated code to bitmap image
- Added `pdebug` macro, which is empty if built with `NDEBUG`
- Added option `--noinline` to suppress all inline code printing
- Added option `-s` to scale output image resolution
- Added debug target to Makefile (`make debug`)
- Changed exclusive options to single `-` and single letter
- Changed printing methods to `pbox` and `praw`
- Changed output messages to use ANSI escape color formatting

## [1.2.2] - 2025-08-03

Add option to force specific Version

- Add option description to help text
- Added `--vnum` option to try to force use of specific code version
- Added trademark disclaimer to runtime copyright header
- Changed `--debug` option name to `--verbose`
- Changed `--silent` option name to `--nocopy`
- Changed argument enum to generate bit flag with `__COUNTER__` macro
- Removed unnecessary literal macros

## [1.2.1] - 2025-07-31

Fix mask penalty calculation

- Added `--mask` option to force program to print a specific mask (e.g.: `--mask 3`)
- Added more debug information when `--debug` is selected
- Fixed mask penalty calculation when comparing columns

## [1.2.0] - 2025-07-16

Add Version 4 and Version 5 QR Codes

- Added Version 4 and 5 codes
- Added script to generate module indexes lookup table
- Changed gcc flags to optimize `make` build
- Moved lookup tables to assembly file `lookup.S`

## [1.1.0] - 2025-07-13

Add Version 3 QR Codes.

- Added Makefile
- Added project information on execution
- Added Version 3 codes
- Added column comparison function for `module_penalty_` method
- Added `--debug` and `--raw` options to command-line
- Added contributing guide and disclaimer to `README.md`
- Added this `CHANGELOG` file
- Changed generated code example on `README.md`
- Changed command-line argument parsing
- Removed `/build` folder
- Fixed `qrmask_t` version argument condition on creation
- Fixed `qrmask_t` uninitialized modules array
- Fixed quiet zone size

## [1.0.0] - 2025-07-09

Initial release.

- Added Version 1 and 2 QR Codes


[Unreleased]: https://github.com/oAGoulart/qrcode/compare/v1.11.0..HEAD
[1.11.0]: https://github.com/oAGoulart/qrcode/releases/tag/v1.11.0
[1.4.0]: https://github.com/oAGoulart/qrcode/releases/tag/v1.4.0
[1.3.0]: https://github.com/oAGoulart/qrcode/releases/tag/v1.3.0
[1.2.2]: https://github.com/oAGoulart/qrcode/releases/tag/v1.2.2
[1.2.1]: https://github.com/oAGoulart/qrcode/releases/tag/v1.2.1
[1.2.0]: https://github.com/oAGoulart/qrcode/releases/tag/v1.2.0
[1.1.0]: https://github.com/oAGoulart/qrcode/releases/tag/v1.1.0
[1.0.0]: https://github.com/oAGoulart/qrcode/releases/tag/v1.0.0
