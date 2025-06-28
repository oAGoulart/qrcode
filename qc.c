#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "hashtables.h"

// WARNING: do not change defined values
#define GEN_MODE 4
#define STRING_MAX 17
#define DATA_LEN STRING_MAX + 2
#define ECC_LEN 7
#define UNICODE_LEN 4
#define BASENUM_DARKMODULES 106
#define BASENUM_LIGHTMODULES 107

static const uint8_t
bitmask[8] = {128,64,32,16,8,4,2,1};

struct qrmask_s {
  uint8_t mask[MATRIX_L1_LEN];
  uint16_t dark_modules;
  uint16_t light_modules;
};

static void
vshift(uint8_t* v, const uint16_t length)
{
  for (uint16_t i = 0; i < length - 1; i++)
  {
    v[i] = v[i + 1];
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
  double percentage = (count / MATRIX_L1_LEN) * 10;
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
  for (; i < MATRIX_L1_ORDER; i++)
  {
    uint16_t row = i * MATRIX_L1_ORDER;
    uint16_t j = 0;
    // Step 1: row direction >>>
    for (; j < MATRIX_L1_ORDER - 1; j++)
    {
      uint8_t* module = &matrix[row + j];
      uint8_t* next = &matrix[row + j + 1];
      if (*module == *next)
      {
        // NOTE: square penalty
        if (i < MATRIX_L1_ORDER - 1) {
          if (*(module + MATRIX_L1_ORDER) == *module &&
              *(next + MATRIX_L1_ORDER) == *module)
          {
            penalty += 3;
          }
        }
        // NOTE: sequential line penalty (row)
        if (j < MATRIX_L1_ORDER - 4)
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
      if (j < MATRIX_L1_ORDER - 10)
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
    for (j = 0; j < MATRIX_L1_LEN - (4 * MATRIX_L1_ORDER); j += MATRIX_L1_ORDER)
    {
      uint8_t* module = &matrix[i + j];
      uint8_t* next = &matrix[i + j + MATRIX_L1_ORDER];
      if (*module == *next)
      {
        // NOTE: sequential line penalty (column)
        uint16_t count = j;
        for (; *(next + j) == *module; j += MATRIX_L1_ORDER);
        count = (uint16_t)floor((j - count) / MATRIX_L1_ORDER);
        if (count > 4)
        {
          penalty += (uint16_t)(3 + (count - 5));
        }
      }
      // NOTE: pattern penalty (column)
      if (j < MATRIX_L1_LEN - (10 * MATRIX_L1_ORDER))
      {
        if (*module == 0 && *next == 0)
        {
          if (*(module + (2 * MATRIX_L1_ORDER)) == 0 &&
              *(module + (3 * MATRIX_L1_ORDER)) == 0 &&
              *(module + (4 * MATRIX_L1_ORDER)) == 1 &&
              *(module + (5 * MATRIX_L1_ORDER)) == 0 &&
              *(module + (6 * MATRIX_L1_ORDER)) == 1 &&
              *(module + (7 * MATRIX_L1_ORDER)) == 1 &&
              *(module + (8 * MATRIX_L1_ORDER)) == 1 &&
              *(module + (9 * MATRIX_L1_ORDER)) == 0 &&
              *(module + (10 * MATRIX_L1_ORDER)) == 1)
          {
            penalty += 40;
          }
        }
        else if (*module == 1 && *next == 0)
        {
          if (*(module + (2 * MATRIX_L1_ORDER)) == 1 &&
              *(module + (3 * MATRIX_L1_ORDER)) == 1 &&
              *(module + (4 * MATRIX_L1_ORDER)) == 1 &&
              *(module + (5 * MATRIX_L1_ORDER)) == 0 &&
              *(module + (6 * MATRIX_L1_ORDER)) == 1 &&
              *(module + (7 * MATRIX_L1_ORDER)) == 0 &&
              *(module + (8 * MATRIX_L1_ORDER)) == 0 &&
              *(module + (9 * MATRIX_L1_ORDER)) == 0 &&
              *(module + (10 * MATRIX_L1_ORDER)) == 0)
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
main(int argc, char* argv[])
{
  if (argc < 2) {
    printf("Usage: qc [string]\r\n"
           "           max:17\r\n");
    return -1;
  }
  // NOTE: byte string, not null-terminated
  char str[STRING_MAX];
  size_t count = strlen(argv[1]);
  count = (count > STRING_MAX) ? STRING_MAX : count;
  memcpy(str, argv[1], count);

  // Step 1: initialize data bits
  //         |mode|    count|                 data|
  //         |0100 0000|0000 0000|0000 0000 . . . |
  //         |byte 1   |byte 2   |byte 3    to 19 |
  uint8_t bitstream[DATA_LEN];
  memset(&bitstream[0], 0, DATA_LEN);
  bitstream[0] = (GEN_MODE << 4) | (uint8_t)(count >> 4);
  bitstream[1] = (uint8_t)count << 4;
  uint16_t i = 0;
  for (; i < count; i++)
  {
    bitstream[i + 1] |= (uint8_t)(str[i] >> 4);
    bitstream[i + 2] |= (uint8_t)(str[i] << 4);
  }

  // Step 2: initialize error-correction codes
  uint8_t ecc[DATA_LEN];
  memcpy(&ecc[0], &bitstream[0], DATA_LEN);

  // Step 3: polynomial division (long division)
  //         using Galois field arithmetic
  for (i = 0; i < DATA_LEN; i++)
  {
    uint8_t lead = ecc[0];
    uint16_t j = 0;
    for (j = 0; j < GENERATOR_L1_LEN; j++)
    {
      ecc[j] ^= alog_ht[(gen1_ht[j] + log_ht[lead]) % UINT8_MAX];
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
    memcpy(&(masks[i].mask[0]), &qr1_ht[0], MATRIX_L1_LEN);
    masks[i].dark_modules = BASENUM_DARKMODULES;
    masks[i].light_modules = BASENUM_LIGHTMODULES;
  }
  
  // Step 6: translate bitstream into modules for each mask
  for (i = 0; i < BYTE_L1_COUNT; i++)
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
        uint8_t mask_module = (mask1_ht[k][index]) ? !module : module;
        masks[k].mask[idx1_ht[index]] = mask_module;
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
    masks[min_score].mask[maskinfo1_ht[0][i]] =
      (maskinfo1[min_score] >> (MASKINFO_LEN - i - 1)) & 1;
    masks[min_score].mask[maskinfo1_ht[1][i]] =
      (maskinfo1[min_score] >> (MASKINFO_LEN - i - 1)) & 1;
  }

  // Step 9: print it unto terminal
  //         use code page 65001 to show block characters
  system("chcp 65001>nul");
  qcshow(&masks[min_score].mask[0], MATRIX_L1_ORDER);
  return 0;
}
