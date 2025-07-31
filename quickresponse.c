#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "shared.h"
#include "mask.h"
#include "quickresponse.h"

#define GEN_MODE 4
#define NUM_PADBITS 7

extern const uint8_t logt[];
extern const uint8_t alogt[];
extern const uint8_t rsgen[];

static void
vshift_(uint8_t* v, uint8_t length)
{
  uint8_t ui8 = 0;
  for (; ui8 < length - 1; ui8++)
  {
    v[ui8] = v[ui8 + 1];
  }
  v[length - 1] = 0;
}

struct qrcode_s
{
  uint8_t* stream_;
  uint8_t slen_;
  uint8_t chosen_;
  qrmask_t* masks_[NUM_MASKS];
  uint8_t version_;
};

int
create_qrcode(qrcode_t** self, char* str, uint8_t should_debug)
{
  const uint8_t bitmask[8] = {1u, 2u, 4u, 8u, 16u, 32u, 64u, 128u};
  const uint8_t strmax[MAX_VERSION] = {17u, 32u, 53u, 78u, 106u};
  const uint8_t ecclen[MAX_VERSION] = {7u, 10u, 15u, 20u, 26u};
  const uint8_t numbytes[MAX_VERSION] = {26u, 44u, 70u, 100u, 134u};

  if (*self != NULL)
  {
    return EINVAL;
  }
  size_t str_count = strlen(str);
  if (str_count > strmax[MAX_VERSION - 1])
  {
    fprintf(stderr, "\tstring must be less than %u characters long\r\n",
            strmax[MAX_VERSION - 1]);
    return EINVAL;
  }
  *self = (qrcode_t*)malloc(sizeof(qrcode_t));
  if (*self == NULL)
  {
    return ENOMEM;
  }
  uint16_t offset = 0;
  uint8_t version = 0;
  for (; version < MAX_VERSION; version++)
  {
    if (str_count <= strmax[version])
    {
      break;
    }
    offset += ecclen[version] + 1;
  }
  const uint8_t* gen = rsgen + offset;
  (*self)->version_ = version;
  const uint8_t data_len = strmax[version] + 2;
  (*self)->slen_ = data_len;
  (*self)->stream_ = (uint8_t*)malloc(data_len);
  if ((*self)->stream_ == NULL)
  {
    free(*self);
    *self = NULL;
    return ENOMEM;
  }
  memset((*self)->stream_, 0, data_len);
  (*self)->stream_[0] = (GEN_MODE << 4) | (uint8_t)(str_count >> 4);
  (*self)->stream_[1] = (uint8_t)(str_count << 4);
  uint8_t ui8 = 0;
  for (; ui8 < str_count; ui8++)
  {
    (*self)->stream_[ui8 + 1] |= (uint8_t)(str[ui8] >> 4);
    (*self)->stream_[ui8 + 2] = (uint8_t)(str[ui8] << 4);
  }

  uint8_t ecc[data_len];
  memcpy(&ecc[0], (*self)->stream_, data_len);
  // NOTE: polynomial division (long division)
  for (ui8 = 0; ui8 < data_len; ui8++)
  {
    uint8_t lead = ecc[0];
    uint8_t uj8 = 0;
    for (; uj8 < ecclen[version] + 1; uj8++)
    {
      ecc[uj8] ^= alogt[(gen[uj8] + logt[lead]) % GF_MAX];
    }
    vshift_(&ecc[0], data_len);
  }
  const uint8_t total_bytes = numbytes[version];
  uint8_t* tmp_ptr = (uint8_t*)realloc((*self)->stream_, total_bytes);
  if (tmp_ptr == NULL)
  {
    free((*self)->stream_);
    free(*self);
    *self = NULL;
    return ENOMEM;
  }
  (*self)->stream_ = tmp_ptr;
  memcpy(&(*self)->stream_[data_len], &ecc[0], ecclen[version]);
  if (should_debug)
  {
    printf("(INFO) Calculated bytes (%d): [%x",
           total_bytes, (*self)->stream_[0]);
    for (ui8 = 1; ui8 < total_bytes; ui8++)
    {
      printf(", %d", (*self)->stream_[ui8]);
    }
    puts("]");
  }

  for (ui8 = 0; ui8 < NUM_MASKS; ui8++)
  {
    (*self)->masks_[ui8] = NULL;
    if (create_qrmask(&(*self)->masks_[ui8], version, ui8) != 0)
    {
      delete_qrcode(self);
      return ENOMEM;
    }
  }
  for (ui8 = 0; ui8 < total_bytes; ui8++)
  {
    offset = ui8 * 8;
    // NOTE: bitstream goes from bit 7 to bit 0
    int8_t bit = 7;
    for (; bit >= 0; bit--)
    {
      uint8_t module = ((*self)->stream_[ui8] & bitmask[bit]) >> bit & 1;
      uint8_t uj8 = 0;
      for (; uj8 < NUM_MASKS; uj8++)
      {
        uint16_t index = (uint16_t)(offset + (7 - bit));
        qrmask_set((*self)->masks_[uj8], index, module);
      }
    }
  }
  // NOTE: padding bits, MUST check xor
  if (version > 0)
  {
    for (ui8 = 0; ui8 < NUM_PADBITS; ui8++)
    {
      uint8_t uj8 = 0;
      for (; uj8 < NUM_MASKS; uj8++)
      {
        uint16_t index = (uint16_t)(total_bytes * 8) + ui8;
        qrmask_set((*self)->masks_[uj8], index, MASK_LIGHT);
      }
    }
  }
  uint16_t curr_score = 0;
  uint16_t min_score = UINT16_MAX;
  uint8_t chosen = 0;
  for (ui8 = 0; ui8 < NUM_MASKS; ui8++)
  {
    curr_score = qrmask_penalty((*self)->masks_[ui8]);
    qrmask_apply((*self)->masks_[ui8]);
    if (curr_score < min_score)
    {
      min_score = curr_score;
      chosen = ui8;
    }
    if (should_debug)
    {
      printf("(INFO) Mask [%d] penalty: %d\r\n", ui8, curr_score);
    }
  }
  (*self)->chosen_ = chosen;
  if (should_debug)
  {
    printf("(INFO) Mask chosen: %d\r\n", chosen);
  }
  return 0;
}

void
delete_qrcode(qrcode_t** self)
{
  if (*self != NULL)
  {
    if ((*self)->stream_ != NULL)
    {
      free((*self)->stream_);
    }
    uint8_t ui8 = 0;
    for (; ui8 < NUM_MASKS; ui8++)
    {
      if ((*self)->masks_[ui8] != NULL)
      {
        delete_qrmask(&(*self)->masks_[ui8]);
      }
    }
    free(*self);
    *self = NULL;
  }
}

void
qrcode_print(qrcode_t* self, uint8_t use_raw, int force_mask)
{
  uint8_t mask = (force_mask >= 0 && force_mask < 8) ?
    (uint8_t)force_mask : self->chosen_;
  if (use_raw)
  {
    qrmask_raw(self->masks_[mask]);
  }
  else
  {
    qrmask_print(self->masks_[mask]);
  }
}
