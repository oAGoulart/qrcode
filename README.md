# Command-line QR Code generator
[![Static Badge](https://img.shields.io/badge/ISO%2FIEC-18004%3A2024-red)](https://www.iso.org/standard/83389.html)
[![GitHub Release](https://img.shields.io/github/v/release/oagoulart/qrcode?color=green)](https://github.com/oAGoulart/qrcode/releases)
[![GitHub License](https://img.shields.io/github/license/oagoulart/qrcode)](https://github.com/oAGoulart/qrcode/tree/master?tab=MS-RL-1-ov-file)
[![DOI](https://zenodo.org/badge/998115592.svg)](https://doi.org/10.5281/zenodo.15851589)

**Objective:** Model 2<sup>(1)</sup>, byte mode, from version 1 to 5, with EC level L. There are no dependencies, generator can be built from code on Windows and Linux.

<sup>(1).</sup> As specified on ISO/IEC 18004:2024.

**Progress:**
- [x] EC code generation
- [x] Masking patterns penalty score calculation
- [x] Selection of minimum version
- [x] Module printing on terminal
- [ ] Version 1 to 5 lookup tables
    - [x] Version 1 (up to 17 characters)
    - [x] Version 2 (up to 32 characters)
    - [x] Version 3 (up to 53 characters)
    - [ ] Version 4 (up to 78 characters)
    - [ ] Version 5 (up to 106 characters)

**Pre-generated lookup tables:**
1. Reed-Solomon EC generator polynomials.
1. Galois field of 256 (285 primitive) log and anti-log table.
1. Placement index of each data and error correction bit on the encoding region.

**Scan tests performed:**
- Version 1 through 3 on iOS 18.5.

## Example

**Command-line:** 
```bash
qrcode --silent oagoulart.github.io/rambles/keep-thyself-credible
```

**Output:**
```bash

  █▀▀▀▀▀█  ▀▀▀██  ▄ ▀ ▄ █▀▀▀▀▀█  
  █ ███ █ ▄▀▄ █▄▀█ ▄  ▄ █ ███ █  
  █ ▀▀▀ █ ▀▀  ▀ ▄ ▀▄█▀▀ █ ▀▀▀ █  
  ▀▀▀▀▀▀▀ ▀ ▀ ▀ █ ▀ █▄▀ ▀▀▀▀▀▀▀  
  ██   ▀▀█▄▀▄▄▀██▄ █▄▄▀▄ ▄██     
  ▄███ ▀▀ █▀  ▀ ▀▀▄ ▄▄█  ▀▀█▀▄▄  
  ▄ █▄▄▀▀ ▀█▀▄▀  █▄ ▄▄▀ ██▄▄  █  
   ▀▄▄█ ▀▀▀  ▄█▀█▀▀  ▄▀▀██▄█▄▄▄  
  ▄▄▀██ ▀█ ██▄▀█ ▄ ▀ █▄▄▄▄▄▄█▀▄  
  █▀▄ ▄▄▀▄█ █▀▄▀▄▀▀ ▀▀ ▄▀█ █▀▄█  
  ▀  ▀▀▀▀▀█ ▄█▀▀██▀██▀█▀▀▀█ █ ▀  
  █▀▀▀▀▀█ █▀  ▀▀▄▀█▀ ██ ▀ ██ ▄   
  █ ███ █  ▀ █▀ ▄██▄▄▀██▀▀▀  ██  
  █ ▀▀▀ █ ▄ █  ▀▄▀▀▀▀▀▀▀█ █▄█▀▄  
  ▀▀▀▀▀▀▀ ▀▀▀ ▀▀▀▀ ▀   ▀▀▀▀▀▀    

```

_As a screenshot:_

![qrcode](./assets/screenshot.png)

## Contributing

All contributions that furthers this project's **objective** (see above) are welcome.
As of right now, this project needs quantitative testing of generated QR codes.

How to contribute with **testing**:
1. Compile the code with `make` (use MinGW or other on Windows);
1. Generate _at least_ two QR codes for each version (see characters capacity above) and try to scan it with your mobile device;
1. Open an issue in this repo with your results (even if all scans succeeded).

How to contribute with **code**:
1. Before opening any PR, create an issue discussing your proposed changes and why they are necessary (e.g. better design pattern);
1. Compile on Windows and Linux before pushing the code;
1. If your code does anything with heap memory, make sure to run it through [Valgrind](https://valgrind.org).

## Disclaimer
QR Code, iQR Code SQRC and FrameQR are registered trademarks of DENSO WAVE INCORPORATED in Japan and in other countries.

ISO/IEC 18004:2024 - Information technology — Automatic identification and data capture techniques — QR Code bar code symbology specification is &copy; ISO/IEC 2024 – All rights reserved.
