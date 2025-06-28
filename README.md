# QR-Code

WIP

NOTE: Only L1 codes (17-bytes maximum)

**Pre-generated hash-tables:**
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
