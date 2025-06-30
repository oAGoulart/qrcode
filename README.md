# Command-line QR-Code generator

**WIP**

**Objective:** Byte mode, from version 1 to 5, with EC level L. Currently, only 1L, but modules are already printed on terminal.

**Progress:**
- [x] EC code generation
- [x] Masking patterns penalty score calculation
- [x] Selection of minimum version
- [x] Module printing on terminal
- [ ] Version 1 to 5 hash tables
    - [x] Version 1 (limited tests performed)
    - [x] Version 2 (detected but not valid)
    - [ ] Version 3
    - [ ] Version 4
    - [ ] Version 5

**Pre-generated hash tables:**
1. ECC generator polynomial.
2. Galois field log and anti-log table.
3. QR-code alignment and timing patterns.
4. Indexes of data/ecc bits on the module matrix.
5. Each of eight masks' XORing pattern.
6. Indexes of mask information bits on the module matrix.

**Input:** "google.com"

**Output:**
```

  █▀▀▀▀▀█ ▀▀ ▄▀ █▀▀▀▀▀█  
  █ ███ █ █▄▀ ▄ █ ███ █  
  █ ▀▀▀ █ ▀ ▄▀  █ ▀▀▀ █  
  ▀▀▀▀▀▀▀ ▀ ▀▄▀ ▀▀▀▀▀▀▀  
  ▀██▀ ▄▀▄█ █ ▄▀ ▄██▀ █  
  ▀▀██▀█▀▄ ▄▀▀ ▀ ▀ ▄▀▀█  
   ▀ ▀▀▀▀▀▄ ▄▄▀▄▀ █ ▄▀   
  █▀▀▀▀▀█  █  ▄▄▀ ▀██ ▄
  █ ███ █ ▄▄▀▀  ▄▀ █▀▄   
  █ ▀▀▀ █ ██▄▄▀▀▄  ▀  ▄
  ▀▀▀▀▀▀▀ ▀▀   ▀ ▀  ▀  

```

_As a screenshot:_

![qrcode](https://i.imgur.com/dgTWO2F.png)
