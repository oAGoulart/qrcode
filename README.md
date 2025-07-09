# Command-line QR-Code generator
![GitHub Release](https://img.shields.io/github/v/release/oagoulart/qrcode?color=green)
![GitHub License](https://img.shields.io/github/license/oagoulart/qrcode)
[![DOI](https://zenodo.org/badge/998115592.svg)](https://doi.org/10.5281/zenodo.15851589)

**WIP**

**Objective:** Byte mode, from version 1 to 5, with EC level L. Currently, only 1L and 2L, but modules are already printed on terminal.

**Progress:**
- [x] EC code generation
- [x] Masking patterns penalty score calculation
- [x] Selection of minimum version
- [x] Module printing on terminal
- [ ] Version 1 to 5 lookup tables
    - [x] Version 1 (limited tests performed)
    - [x] Version 2 (limited tests performed)
    - [ ] Version 3
    - [ ] Version 4
    - [ ] Version 5

**Pre-generated lookup tables:**
1. ECC generator polynomial.
1. Galois field log and anti-log table.
1. Indexes of data/ecc bits on the module matrix.

**Input:** "oagoulart.github.io/"

**Output:**
```

  █▀▀▀▀▀█ ▀▀▀▀█▄█   █▀▀▀▀▀█
  █ ███ █  ▀█ ██▀▄▄ █ ███ █
  █ ▀▀▀ █  ▀▄████ ▄ █ ▀▀▀ █
  ▀▀▀▀▀▀▀ █ ▀ █▄▀▄▀ ▀▀▀▀▀▀▀
  ▀█▄▀█▄▀ ▄▀█ █▄▄█▀ ▀ ▄▄▄ ▀
  ▀  ▀▄▀▀▀▄▀▄██▀▄▀▀▄ ▄▀▄█ █
  ▄█ ▀▀▀▀█▀█▀▀████▄ ▀ ▄█▄▀▀
  █▀█▄▄▀▀ ██  █▀▄▀██▀▀█▄▄█▀
  ▀ ▀▀▀▀▀▀▄▀▄██ ▀ █▀▀▀█▄█▄ 
  █▀▀▀▀▀█  █▄▀███▄█ ▀ █▄▀ █
  █ ███ █ █ ▀ █▀▄▀████▀█ ██
  █ ▀▀▀ █ ▄▄▄██ ▀▄ █ █▀█▄██
  ▀▀▀▀▀▀▀ ▀▀▀▀▀▀▀ ▀ ▀▀▀▀  ▀

```

_As a screenshot:_

![qrcode](./assets/screenshot.png)
