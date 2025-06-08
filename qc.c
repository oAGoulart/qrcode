#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// WARNING: do not change defined values
#define GEN_MODE 4
#define STRING_MAX 17
#define DATA_LEN STRING_MAX + 2
#define ECC_LEN 7
#define GENERATOR_LEN 8
#define LOGTABLE_LEN 256
#define MATRIX_ORDER 21
#define MATRIX_COUNT MATRIX_ORDER * MATRIX_ORDER
#define BYTE_COUNT 26
#define BIT_COUNT BYTE_COUNT * 8
#define UNICODE_LEN 4
#define BASENUM_DARKMODULES 106
#define BASENUM_LIGHTMODULES 107
#define NUM_MASKS 8
#define MASKINFO_LEN 15

// NOTE: only for L1 QR-codes
static const uint8_t
gen_ht[GENERATOR_LEN] = {
  0,87,229,146,149,238,102,21
};

static const uint8_t
antilog_ht[LOGTABLE_LEN] = {
  1,2,4,8,16,32,64,128,29,58,116,232,205,135,19,38,
  76,152,45,90,180,117,234,201,143,3,6,12,24,48,96,192,
  157,39,78,156,37,74,148,53,106,212,181,119,238,193,159,35,
  70,140,5,10,20,40,80,160,93,186,105,210,185,111,222,161,
  95,190,97,194,153,47,94,188,101,202,137,15,30,60,120,240,
  253,231,211,187,107,214,177,127,254,225,223,163,91,182,113,226,
  217,175,67,134,17,34,68,136,13,26,52,104,208,189,103,206,
  129,31,62,124,248,237,199,147,59,118,236,197,151,51,102,204,
  133,23,46,92,184,109,218,169,79,158,33,66,132,21,42,84,
  168,77,154,41,82,164,85,170,73,146,57,114,228,213,183,115,
  230,209,191,99,198,145,63,126,252,229,215,179,123,246,241,255,
  227,219,171,75,150,49,98,196,149,55,110,220,165,87,174,65,
  130,25,50,100,200,141,7,14,28,56,112,224,221,167,83,166,
  81,162,89,178,121,242,249,239,195,155,43,86,172,69,138,9,
  18,36,72,144,61,122,244,245,247,243,251,235,203,139,11,22,
  44,88,176,125,250,233,207,131,27,54,108,216,173,71,142,1
};

// NOTE: index 0 is illegal
static const uint8_t
log_ht[LOGTABLE_LEN] = {
  0,0,1,25,2,50,26,198,3,223,51,238,27,104,199,75,
  4,100,224,14,52,141,239,129,28,193,105,248,200,8,76,113,
  5,138,101,47,225,36,15,33,53,147,142,218,240,18,130,69,
  29,181,194,125,106,39,249,185,201,154,9,120,77,228,114,166,
  6,191,139,98,102,221,48,253,226,152,37,179,16,145,34,136,
  54,208,148,206,143,150,219,189,241,210,19,92,131,56,70,64,
  30,66,182,163,195,72,126,110,107,58,40,84,250,133,186,61,
  202,94,155,159,10,21,121,43,78,212,229,172,115,243,167,87,
  7,112,192,247,140,128,99,13,103,74,222,237,49,197,254,24,
  227,165,153,119,38,184,180,124,17,68,146,217,35,32,137,46,
  55,63,209,91,149,188,207,205,144,135,151,178,220,252,190,97,
  242,86,211,171,20,42,93,158,132,60,57,83,71,109,65,162,
  31,45,67,216,183,123,164,118,196,23,73,236,127,12,111,246,
  108,161,59,82,41,157,85,170,251,96,134,177,187,204,62,90,
  203,89,95,176,156,169,160,81,11,245,22,235,122,117,44,215,
  79,174,213,233,230,231,173,232,116,214,244,234,168,80,88,175
};

static const uint8_t
qr_ht[MATRIX_COUNT] = {
  1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,
  1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,1,0,1,1,1,0,1,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,1,0,1,1,1,0,1,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,1,0,1,1,1,0,1,
  1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,
  1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
  1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

// NOTE: index of each module on the qr_ht array,
//       placement starts from the bottom-right corner
static const uint16_t
index_ht[BIT_COUNT] = {
  440,439,419,418,398,397,377,376,356,355,335,334,
  314,313,293,292,272,271,251,250,230,229,209,208, // top
  207,206,228,227,249,248,270,269,291,290,312,311,
  333,332,354,353,375,374,396,395,417,416,438,437, // bottom
  436,435,415,414,394,393,373,372,352,351,331,330,
  310,309,289,288,268,267,247,246,226,225,205,204, // top
  203,202,224,223,245,244,266,265,287,286,308,307,
  329,328,350,349,371,370,392,391,413,412,434,433, // bottom
  432,431,411,410,390,389,369,368,348,347,327,326,
  306,305,285,284,264,263,243,242,222,221,201,200,
  180,179,159,158,117,116,96,95,75,74,54,53,33,32,12,11, // top
  10,9,31,30,52,51,73,72,94,93,115,114,157,156,178,
  177,199,198,220,219,241,240,262,261,283,282,304,
  303,325,324,346,345,367,366,388,387,409,408,430,429, // bottom
  260,259,239,238,218,217,197,196, // top
  194,193,215,214,236,235,257,256, // bottom
  255,254,234,233,213,212,192,191, // top
  190,189,211,210,232,231,253,252 // bottom
};

// NOTE: mask is only applied to data and ecc bits
static const uint8_t
mask_ht[NUM_MASKS][BIT_COUNT] = {
  {
    1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,0,1,1,0,0,1,1,0,0,1,1,0,0,1,
    1,0,0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,0,1,1,0,
    0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,
    0,1,1,0,0,1,1,0,0,1,0,1,1,0,0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,1,0,0,1,0,1,1,0,
    0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,1,0,0,1,1,0,
    0,1,0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1
  },
  {
    1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,
    1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,1,1,
    0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,
    0,0,1,1,0,0,1,1,0,0,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0,0,0,1,1,
    0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0,0,0,1,1,0,0,
    1,1,1,1,0,0,1,1,0,0,0,0,1,1,0,0,1,1
  },
  {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
    1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
    1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
    0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1
  },
  {
    0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,
    0,1,0,0,1,0,0,1,0,0,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,0,0,1,0,
    0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,
    0,0,0,1,1,0,0,0,0,1,0,0,0,1,1,0,0,0,0,1,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,
    0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,0,1,1,0,0,0,0,0,1,0,0,1,
    0,0,1,0,0,0,0,1,1,0,0,1,0,0,1,0,0,1
  },
  {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,
    1,0,1,0,0,0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,
    0,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,
    0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,1
  },
  {
    0,1,1,0,0,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,1,0,
    0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,
    0,0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,1,0,
    0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,1,0,
    0,0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,
    0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1
  },
  {
    0,1,1,0,1,0,0,0,1,1,1,1,0,1,1,0,1,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,1,0,1,0,
    0,1,1,1,1,1,0,0,1,0,1,1,1,1,0,1,1,0,1,0,0,0,1,1,1,1,0,1,1,0,1,0,0,0,1,1,1,1,
    0,0,1,0,1,0,0,1,1,1,1,1,0,0,1,0,1,0,0,1,1,0,0,0,1,1,1,1,0,1,1,0,1,0,0,0,1,1,
    1,1,0,1,1,0,1,0,0,0,1,1,0,1,1,0,1,0,0,0,1,1,0,1,1,1,1,1,0,0,1,0,1,0,1,1,1,1,
    0,0,1,0,1,0,0,1,1,1,1,1,0,0,1,0,1,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,1,1,1,1,
    1,0,1,1,0,0,0,1,0,1,1,1,1,0,0,1,0,1
  },
  {
    0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,1,0,0,0,0,1,0,1,1,1,1,0,1,
    0,0,0,0,1,0,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,0,0,1,0,
    1,1,1,1,0,1,0,0,0,0,1,0,1,1,1,1,0,1,0,0,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,
    0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,1,0,1,1,1,1,0,1,0,0,1,0,
    1,1,1,1,0,1,0,0,0,0,1,0,1,1,1,1,0,1,0,0,0,0,1,0,1,1,1,1,1,0,0,0,1,1,0,1,0,0,
    0,0,0,1,1,1,1,1,1,0,0,0,0,0,1,0,1,1
  }
};

// NOTE: 15 bits each
static const uint16_t
maskinfo[NUM_MASKS] = {
  30660,29427,32170,30877,26159,25368,27713,26998
};

// NOTE: maskinfo is duplicated into two different positions
static const uint16_t
maskinfo_ht[2][MASKINFO_LEN] = {
  {
    168,169,170,171,172,173,175,176,
    155,113,92,71,50,29,8
  },
  {
    428,407,386,365,344,323,302,
    181,182,183,184,185,186,187,188
  }
};

static const uint8_t
bitmask[8] = {
  128,64,32,16,8,4,2,1
};

struct qrmask_s {
  uint8_t mask[MATRIX_COUNT];
  uint16_t dark_modules;
  uint16_t light_modules;
};

static void
vshift(uint8_t* v, const uint16_t length)
{
  for (uint16_t i = 0; i < length - 1; i++)
  {
    v[i] = v[i+1];
  }
  v[length - 1] = 0;
}

static void
qcdouble(const uint8_t* v, const uint16_t order)
{
  char str[(order * UNICODE_LEN) + 1];
  uint16_t top = 0;
  uint16_t bottom = 0;
  uint16_t str_index = 0;
  for (bottom = order; top < order; top++, bottom++)
  {
    uint8_t ch_case = (uint8_t)(v[top] << 1) + v[bottom];
    switch (ch_case)
    {
    case 0:
      strcpy(&str[str_index], " ");
      str_index += 3;
      break;
    case 1:
      strcpy(&str[str_index], "▄");
      str_index += 3;
      break;
    case 2:
      strcpy(&str[str_index], "▀");
      str_index += 3;
      break;
    default:
      strcpy(&str[str_index], "█");
      str_index += 3;
      break;
    }
  }
  if (str_index > 0)
  {
    str[str_index + 1] = '\0';
    printf("  %s  \r\n", str);
  }
}

static void
qcsingle(const uint8_t* v, const uint16_t order)
{
  char str[(order * UNICODE_LEN) + 1];
  uint16_t top = 0;
  uint16_t str_index = 0;
  for (; top < order; top++)
  {
    if (v[top] == 0)
    {
      strcpy(&str[str_index], " ");
      str_index += 3;
    }
    else
    {
      strcpy(&str[str_index], "▀");
      str_index += 3;
    }
  }
  if (str_index > 0)
  {
    str[str_index + 1] = '\0';
    printf("  %s  \r\n", &str[0]);
  }
}

void
qcshow(const uint8_t* m, const uint16_t order)
{
  puts("");
  uint16_t line = 0;
  for (; line < order - 1; line += 2)
  {
    qcdouble(&m[line * order], order);
  }
  if (order % 2 != 0)
  {
    qcsingle(&m[(order - 1) * order], order);
  }
  puts("");
}

static uint16_t
percentage_penalty(uint16_t count)
{
  double percentage = (count / MATRIX_COUNT) * 10;
  double prev = floor(percentage) * 10;
  double next = percentage - fmod(percentage * 10, 5.0) + 5;
  prev = fabs(prev - 50) / 5;
  next = fabs(next - 50) / 5;
  return (uint8_t)fmin(prev, next) * 10;
}

static uint16_t
module_penalty(uint8_t* matrix)
{
  uint16_t penalty = 0;
  uint16_t i = 0;
  for (; i < MATRIX_ORDER; i++)
  {
    uint16_t row = i * MATRIX_ORDER;
    uint16_t j = 0;
    // Step 1: row direction >>>
    for (; j < MATRIX_ORDER - 1; j++)
    {
      uint8_t* module = &matrix[row + j];
      uint8_t* next = &matrix[row + j + 1];
      if (*module == *next)
      {
        // NOTE: square penalty
        if (i < MATRIX_ORDER - 1) {
          if (*(module + MATRIX_ORDER) == *module &&
              *(next + MATRIX_ORDER) == *module)
          {
            penalty += 3;
          }
        }
        // NOTE: sequential line penalty (row)
        if (j < MATRIX_ORDER - 4)
        {
          uint16_t count = j;
          for (; *(next + j + 1) == *module; j++);
          count = j - count;
          if (count > 4)
          {
            penalty += (uint16_t)(3 + (count - 5));
          }
        }
      }
      // NOTE: pattern penalty (row)
      if (j < MATRIX_ORDER - 10)
      {
        if (*module == 0 && *next == 0)
        {
          if (*(module + 2) == 0 && *(module + 3) == 0 &&
              *(module + 4) == 1 && *(module + 5) == 0 &&
              *(module + 6) == 1 && *(module + 7) == 1 &&
              *(module + 8) == 1 && *(module + 9) == 0 &&
              *(module + 10) == 1)
          {
            penalty += 40;
          }
        }
        else if (*module == 1 && *next == 0)
        {
          if (*(module + 2) == 1 && *(module + 3) == 1 &&
              *(module + 4) == 1 && *(module + 5) == 0 &&
              *(module + 6) == 1 && *(module + 7) == 0 &&
              *(module + 8) == 0 && *(module + 9) == 0 &&
              *(module + 10) == 0)
          {
            penalty += 40;
          }
        }
      }
    }
    // Step 2: column direction vvv
    for (j = 0; j < MATRIX_COUNT - (4 * MATRIX_ORDER); j += MATRIX_ORDER)
    {
      uint8_t* module = &matrix[i + j];
      uint8_t* next = &matrix[i + j + MATRIX_ORDER];
      if (*module == *next)
      {
        // NOTE: sequential line penalty (column)
        uint16_t count = j;
        for (; *(next + j) == *module; j += MATRIX_ORDER);
        count = (uint16_t)floor((j - count) / MATRIX_ORDER);
        if (count > 4)
        {
          penalty += (uint16_t)(3 + (count - 5));
        }
      }
      // NOTE: pattern penalty (column)
      if (j < MATRIX_COUNT - (10 * MATRIX_ORDER))
      {
        if (*module == 0 && *next == 0)
        {
          if (*(module + (2 * MATRIX_ORDER)) == 0 &&
              *(module + (3 * MATRIX_ORDER)) == 0 &&
              *(module + (4 * MATRIX_ORDER)) == 1 &&
              *(module + (5 * MATRIX_ORDER)) == 0 &&
              *(module + (6 * MATRIX_ORDER)) == 1 &&
              *(module + (7 * MATRIX_ORDER)) == 1 &&
              *(module + (8 * MATRIX_ORDER)) == 1 &&
              *(module + (9 * MATRIX_ORDER)) == 0 &&
              *(module + (10 * MATRIX_ORDER)) == 1)
          {
            penalty += 40;
          }
        }
        else if (*module == 1 && *next == 0)
        {
          if (*(module + (2 * MATRIX_ORDER)) == 1 &&
              *(module + (3 * MATRIX_ORDER)) == 1 &&
              *(module + (4 * MATRIX_ORDER)) == 1 &&
              *(module + (5 * MATRIX_ORDER)) == 0 &&
              *(module + (6 * MATRIX_ORDER)) == 1 &&
              *(module + (7 * MATRIX_ORDER)) == 0 &&
              *(module + (8 * MATRIX_ORDER)) == 0 &&
              *(module + (9 * MATRIX_ORDER)) == 0 &&
              *(module + (10 * MATRIX_ORDER)) == 0)
          {
            penalty += 40;
          }
        }
      }
    }
  }
  return penalty;
}

int
main()
{
  // change to code page 65001 to show block characters
  system("chcp 65001>nul");

  // TODO: get string from user input
  const char* str = "google.com";
  size_t count = strlen(str);
  if (count > STRING_MAX)
  {
    return -1;
  }

  // Step 1: initialize data bits
  //         |mode|    count|                 data|
  //         |0100 0000|0000 0000|0000 0000 . . . |
  //         |byte 1   |byte 2   |byte 3    to 19 |
  uint8_t bitstream[DATA_LEN];
  bitstream[0] = (GEN_MODE << 4) | (uint8_t)(count >> 4);
  bitstream[1] = (uint8_t)count << 4;
  uint16_t i = 0;
  for (; i < count; i++)
  {
    bitstream[i + 1] |= (uint8_t)str[i] >> 4;
    bitstream[i + 2] |= (uint8_t)str[i] << 4;
  }
  memset(&bitstream[i + 2], 0, DATA_LEN - (i + 2));

  // Step 2: initialize error-correction codes
  uint8_t ecc[DATA_LEN];
  memcpy(&ecc[0], &bitstream[0], DATA_LEN);

  // Step 3: polynomial division (long division)
  //         using Galois field arithmetic
  for (i = 0; i < DATA_LEN; i++)
  {
    uint8_t lead = ecc[0];
    uint16_t j = 0;
    for (j = 0; j < GENERATOR_LEN; j++)
    {
      ecc[j] ^= antilog_ht[(gen_ht[j] + log_ht[lead]) % UINT8_MAX];
    }
    if (ecc[0] == 0)
    {
      // NOTE: if multiplier is zero, then discard it
      vshift(&ecc[0], DATA_LEN);
    }
  }
  
  // Step 4: concatenate data and error-correction codes
  uint8_t final_bits[DATA_LEN + ECC_LEN];
  memcpy(&final_bits[0], &bitstream[0], DATA_LEN);
  memcpy(&final_bits[DATA_LEN], &ecc[0], ECC_LEN);

  // Step 5: initialize masks
  struct qrmask_s masks[NUM_MASKS];
  for (i = 0; i < NUM_MASKS; i++)
  {
    memcpy(&(masks[i].mask[0]), &qr_ht[0], MATRIX_COUNT);
    masks[i].dark_modules = BASENUM_DARKMODULES;
    masks[i].light_modules = BASENUM_LIGHTMODULES;
  }
  
  // Step 6: translate bitstream into modules for each mask
  for (i = 0; i < BYTE_COUNT; i++)
  {
    uint16_t offset = i * 8;
    // NOTE: bitstream goes from bit 7 to bit 0
    int j = 7;
    for (; j >= 0; j--)
    {
      uint8_t module = (final_bits[i] & bitmask[7 - j]) >> j & 1;
      uint8_t k = 0;
      for (; k < NUM_MASKS; k++)
      {
        uint16_t index = (uint16_t)(offset + (7 - j));
        uint8_t mask_module = (mask_ht[k][index]) ? !module : module;
        masks[k].mask[index_ht[index]] = mask_module;
        if (mask_module == 0)
        {
          masks[k].light_modules++;
        }
        else
        {
          masks[k].dark_modules++;
        }
      }
    }
  }

  // Step 7: calculate each mask's penalty score
  uint16_t final_scores[NUM_MASKS];
  uint16_t min_score = 0;
  for (i = 0; i < NUM_MASKS; i++)
  {
    final_scores[i] = 0;
    // NOTE: 1st, 2nd and 3rd penalty scores
    final_scores[i] += module_penalty(&masks[i].mask[0]);
    // NOTE: 4th penalty score
    final_scores[i] += percentage_penalty(masks[i].dark_modules);
    if (final_scores[i] < final_scores[min_score])
    {
      min_score = i;
    }
  }

  // Step 8: add version info
  for (i = 0; i < MASKINFO_LEN; i++)
  {
    masks[min_score].mask[maskinfo_ht[0][i]] =
      (maskinfo[min_score] >> (MASKINFO_LEN - i - 1)) & 1;
    masks[min_score].mask[maskinfo_ht[1][i]] =
      (maskinfo[min_score] >> (MASKINFO_LEN - i - 1)) & 1;
  }

  // Step 9: print it unto terminal
  qcshow(&masks[min_score].mask[0], MATRIX_ORDER);
  return 0;
}
