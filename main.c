#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "hashtables.h"
#include "mask.h"

#define GEN_MODE 4

static const uint8_t
bitmask[8] = {128,64,32,16,8,4,2,1};

static void
vshift(uint8_t* v, const uint16_t length)
{
  for (uint16_t i = 0; i < length - 1; i++)
  {
    v[i] = v[i + 1];
  }
  v[length - 1] = 0;
}

int
main(int argc, char* argv[])
{
  if (argc < 2) {
    printf("Usage: qc [string]\r\n"
           "           max:17\r\n");
    return -1;
  }
  // WARNING: uncomment line below on Windows
  //system("chcp 65001>nul");

  const uint8_t strmax = qr_strmax(0);
  // NOTE: byte string, not null-terminated
  char str[strmax];
  size_t count = strlen(argv[1]);
  count = (count > strmax) ? strmax : count;
  memcpy(str, argv[1], count);

  // Step 1: initialize data bits
  //         |mode|    count|                 data|
  //         |0100 0000|0000 0000|0000 0000 . . . |
  //         |byte 1   |byte 2   |byte 3    to  n |
  const uint8_t datalen = strmax + 2;
  uint8_t bstream[datalen];
  memset(&bstream[0], 0, datalen);
  bstream[0] = (GEN_MODE << 4) | (uint8_t)(count >> 4);
  bstream[1] = (uint8_t)count << 4;
  uint16_t i = 0;
  for (; i < count; i++)
  {
    bstream[i + 1] |= (uint8_t)(str[i] >> 4);
    bstream[i + 2] |= (uint8_t)(str[i] << 4);
  }

  // Step 2: initialize error-correction codes
  uint8_t ecc[datalen];
  memcpy(&ecc[0], &bstream[0], datalen);

  // Step 3: polynomial division (long division)
  //         using Galois field arithmetic
  for (i = 0; i < datalen; i++)
  {
    uint8_t lead = ecc[0];
    uint8_t j = 0;
    for (j = 0; j < qr_ecclen(0) + 1; j++)
    {
      ecc[j] ^= gf_alog((uint8_t)((qr_eccgen(0, j) + gf_log(lead)) % GF_MAX));
    }
    if (ecc[0] == 0)
    {
      // NOTE: if multiplier is zero, then discard it
      vshift(&ecc[0], datalen);
    }
  }
  
  // Step 4: concatenate data and error-correction codes
  const uint8_t total_bytes = qr_streambytes(0);
  uint8_t final_stream[total_bytes];
  memcpy(&final_stream[0], &bstream[0], datalen);
  memcpy(&final_stream[datalen], &ecc[0], qr_ecclen(0));

  // Step 5: initialize masks
  qrmask_t* masks[NUM_MASKS];
  for (i = 0; i < NUM_MASKS; i++)
  {
    masks[i] = NULL;
    create_qrmask(&masks[i], 1, (uint8_t)i);
  }
  
  // Step 6: translate bitstream into modules for each mask
  for (i = 0; i < total_bytes; i++)
  {
    uint16_t offset = i * 8;
    // NOTE: bitstream goes from bit 7 to bit 0
    int j = 7;
    for (; j >= 0; j--)
    {
      uint8_t module = (final_stream[i] & bitmask[7 - j]) >> j & 1;
      uint8_t k = 0;
      for (; k < NUM_MASKS; k++)
      {
        uint16_t index = (uint16_t)(offset + (7 - j));
        qrmask_set(masks[k], index, module);
      }
    }
  }

  // Step 7: calculate each mask's penalty score
  int curr_score = 0;
  int min_score = UINT16_MAX;
  uint8_t chosen = 0;
  for (i = 0; i < NUM_MASKS; i++)
  {
    curr_score = qrmask_penalty(masks[i]);
    if (curr_score < min_score)
    {
      min_score = curr_score;
      chosen = (uint8_t)i;
    }
  }

  // Step 8: add version info and print
  qrmask_apply(masks[chosen]);
  qrmask_print(masks[chosen]);

  for (i = 0; i < NUM_MASKS; i++)
  {
    delete_qrmask(&masks[i]);
  }
  return 0;
}
