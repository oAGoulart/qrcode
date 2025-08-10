# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

- Added option `--bmp` to write generated code to bitmap image
- Added `pdebug` macro, which is empty if built with `NDEBUG`
- Added option `--noinline` to suppress all inline code printing
- Changed printing methods to `pbox` and `praw`
- Changed output messages to use ANSI escape color formatting

## [1.2.2] - 2025-08-03

Add option to force specific Version

- Added option description to help text
- Added `--vnum` option to try to force use of specific code version
- Added trademark disclaimer to runtime copyright header
- Changed `--debug` option name to `--verbose`
- Changed `--silent` option name to `--nocopy`
- Changed argument enum to generate bit flag with `__COUNTER__` macro
- Removed unnecessary literal macros

## [1.2.1] - 2025-07-31

Fixed mask penalty calculation

- Added `--mask` option to force program to print a specific mask (e.g.: `--mask 3`)
- Added more debug information when `--debug` is selected
- Fixed mask penalty calculation when comparing columns

## [1.2.0] - 2025-07-16

Added Version 4 and Version 5 QR Codes

- Added Version 4 and 5 codes
- Added script to generate module indexes lookup table
- Changed gcc flags to optimize `make` build
- Moved lookup tables to assembly file `lookup.S`

## [1.1.0] - 2025-07-13

Added Version 3 QR Codes.

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


[Unreleased]: https://github.com/oAGoulart/qrcode/compare/v1.2.2..HEAD
[1.2.2]: https://github.com/oAGoulart/qrcode/releases/tag/v1.2.2
[1.2.1]: https://github.com/oAGoulart/qrcode/releases/tag/v1.2.1
[1.2.0]: https://github.com/oAGoulart/qrcode/releases/tag/v1.2.0
[1.1.0]: https://github.com/oAGoulart/qrcode/releases/tag/v1.1.0
[1.0.0]: https://github.com/oAGoulart/qrcode/releases/tag/v1.0.0
