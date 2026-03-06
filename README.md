# Command-line QR Code generator
[![Static Badge](https://img.shields.io/badge/ISO%2FIEC-18004%3A2024-red)](https://www.iso.org/standard/83389.html)
[![GitHub Release](https://img.shields.io/github/v/release/oagoulart/qrcode?color=green)](https://github.com/oAGoulart/qrcode/releases)
[![GitHub License](https://img.shields.io/badge/license-MS--RL-orange)](https://github.com/oAGoulart/qrcode/tree/master?tab=MS-RL-1-ov-file)
[![DOI](https://zenodo.org/badge/998115592.svg)](https://doi.org/10.5281/zenodo.15851589)

**Features:** Model 2 byte/num/alpha modes, with EC level L, as specified in ISO/IEC 18004:2024.
There are no dependencies, generator can be built from code on any platform. Outputs inline (stdout), bitmap, or vectors (svg).

**Available generation and capacity (codewords):**

|              **Version** | 1  | 2  | 3  | 4  | 5  |
|-------------------------:|:--:|:--:|:--:|:--:|:--:|
|   EC Level **Low** (7%) | 17 | 32 | 53 | 78 | 106 |

> [!NOTE]
> Generator will auto-select lowest Version possible.
> Unless option `-V` is specified.\
> Each **codeword** refers to a full byte (8 bits).\
> Numeric encoding uses up to 10-bits for 3 digits (~58% reduction).\
> Alphanumeric encoding uses up to 11-bits for each pair of characters (~31% reduction).

**Pre-generated lookup tables:**
1. Reed-Solomon error correction generator polynomials.
2. Galois field of 256 (285 primitive) log and anti-log table.
3. Placement index of each module onto the encoding region.
4. QR Code Version/Level information table.

## Usage

**Options available:**

```text
Usage: qrcode [OPTIONS] <data to encode>
OPTIONS:
  -h, --help     show this help message
  --nolimit      ignore inline Version limit (for larger terminals)
  --optimize     reduce data size, encode numeric, alphanumeric, byte
                   segments separately (if any)
  --raw          print generated code with chars 1, 0 (no box-chars)
  -v, --version  show generator's version and build information
  -g <uint>      level of on-screen information <0-3>
  -l <char>      use a specific error correction level (l, m, q, or h)
  -m <uint>      force use of mask <0-7>, regardless of penalty
  -s <uint>      scale image output <1-30> times
  -V <uint>      force use of version <1-5> code (or lower, if
                   used with --optimize)
  -B <string>    create bitmap file with generated code
  -K <string>    create scalable vector image, disregards -s
```

### Building and binaries

Latest (stable) release can be found at this repo's [Releases](https://github.com/oAGoulart/qrcode/releases).
To build this project, use `make`. Otherwise, as long as your system has
Clang and Python, you may run Makefile's `build` commands manually.

> [!CAUTION]
> **Beware 1:** This repo's `master` branch may or may not contain uncompilable, unstable code.
> It's recommended that you select the latest tag release,
> unless you wish to collaborate with code.
>
> **Beware 2:** Whilst building, scripts will generate +2MB of pre-calculated lookup tables. That process should take less than 10s.
>
> **Beware 3:** If using LUT script in debug, make sure `matplotlib` is properly installed. If needed, activate a virtual envionment before executing `make`. With MinGW, make sure to use `pacman` instead of `pip`.

**Build tools used for binaries:**

|     Tool | Ubuntu 22.04<br>amd64/x86<br>(WSL 2) | Windows 11<br>amd64<br>(MSYS2) | FreeBSD 14.3<br>x86<br>(VM) | macOS 15<br>aarch64<br>(VM) | OmniOS r151054<br>amd64<br>(VM) |
|---------:| :----: | :----: | :----: | :----: | :----: |
| GNU Make | 4.3 | 4.4.1 | 4.4.1* | 4.4.1* | 4.4.1* |
|    Clang | 14.0.0 | 21.1.8 | 19.1.7 | 17.0.0 | 20.1.7 |
|   Python | 3.10.12 | 3.14.3 | 3.11.13 | 3.14.3 | 3.13.3 |
| *status* | 🟩 | 🟩 | 🟩 | 🟩 |  🟩 |

\* Use `gmake` instead of system-provided `make`.

### Examples

#### 1. Inline printing

**Command-line:** 
```bash
qrcode oagoulart.github.io/rambles/keep-thyself-credible
```

**Output:**
```bash

    █▀▀▀▀▀█ ▄█  █▄▀▄█▀▀▀█ █▀▀▀▀▀█
    █ ███ █ ▄█  █▄█▀▄▄    █ ███ █
    █ ▀▀▀ █ ▄█  ▀ ▄▄▀▄█▀▀ █ ▀▀▀ █
    ▀▀▀▀▀▀▀ ▀▄▀ ▀ █▄▀ █▄▀ ▀▀▀▀▀▀▀
    █████ ▀█▀█ ▄▀█▀ ▄█▄▄█▀▄█▄█▄█▄
    ▄██▀ ▀▀ ██  ▀ ▀█▄ ▄▄█▄ ▀▀█▀ ▄
    ▄▀▄ █ ▀▀ ▀ █▀▀▀▀█▀▄█ ▄▄▄▄█▀▄▄
     ▀  ▀ ▀▀█▄▄▄█▀▀██  ▄██▀█▄█   
    ▄▄▀▀█ ▀█ ▀█▄▀█   ▀ █▄ ▄▄▄▄██▄
    █ █▄██▀█▄▄▄ ▄ ██ ▀▀ ▀  ▄ ▄  ▄
    ▀  ▀▀▀▀▀█▄ █▀▀▀▀███▀█▀▀▀█ ▀▄█
    █▀▀▀▀▀█ ▀█  ▀▀▄██▀ ██ ▀ ██   
    █ ███ █ ██▀▄▀▀█▀▄█▄ █▀▀▀▀▀▀▀▄
    █ ▀▀▀ █ █▄▀  ▀ ██▀▀▀██▀ █▄▀█ 
    ▀▀▀▀▀▀▀ ▀▀▀ ▀▀▀▀ ▀   ▀▀▀▀▀▀  

```

> [!TIP]
> Using a `line-height` of `1em` (i.e. line height is equal to font size)
> will remove the unintended vertical spacing seen in the output above.
> Some fonts may require a slight different value between `1 +/- .25`.
> You can also try value `normal`.

#### 2. Bitmap generation

**Command-line:** 
```bash
qrcode -s 6 -B gen.bmp oagoulart.github.io/rambles/keep-thyself-credible
```

**Output:**

![QR Code](assets/gen.bmp)

#### 3. Vector graphics generation

**Command-line:** 
```bash
qrcode -K gen.svg oagoulart.github.io/rambles/keep-thyself-credible
```

**Output:**

![QR Code](assets/gen.svg)

### Module path visualizer

If you wish to visualize how modules are placed onto the encoding region, you can use the generator script at `scripts/indexes.py` in debug mode. An usage example with MinGW is:

```sh
pacman -S mingw-w64-x86_64-python-matplotlib
python -m venv .venv --system-site-packages
source .venv/bin/activate
python scripts/indexes.py 2 True
```
Where `2` is the bar code Version and `True` is the debug flag (default: `False`).

![Module path](assets/qrcode2path.png)

## Roadmap

Work-in-progress:
1. higher EC levels
   - [x] redundant data type
   - [ ] split codewords into blocks/group
   - [x] lookup tables
2. higher Version codes
   - [ ] higher EC levels (see above)
   - [x] lookup tables

## Disclaimer
QR Code, iQR Code SQRC and FrameQR are registered trademarks
of DENSO WAVE INCORPORATED in Japan and in other countries.

ISO/IEC 18004:2024 - Information technology — Automatic identification and
data capture techniques — QR Code bar code symbology specification is &copy;
ISO/IEC 2024 – All rights reserved.

## Further reading

1. [ISO/IEC 18004:2024 documentation (purchase)](https://www.iso.org/standard/83389.html)
2. [Denso's QR Code FAQ](https://www.qrcode.com/en/faq.html)
3. [BMP file format](https://gibberlings3.github.io/iesdp/file_formats/ie_formats/bmp.htm)
4. [Scalable Vector Graphics (SVG) 1.1 (Second Edition)](https://www.w3.org/TR/SVG11/)


