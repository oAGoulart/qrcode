#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "shared.h"
#include "mask.h"
#include "quickresponse.h"

#define GEN_MODE 4

static const uint8_t
bitmask_[8] = {1,2,4,8,16,32,64,128};

static const uint8_t
alog_[GF_MAX + 1] = {
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
log_[GF_MAX + 1] = {
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
strmax_[MAX_VERSION] = {17,32,53,78,106};
static const uint8_t
ecclen_[MAX_VERSION] = {7,10,15,20,26};
static const uint8_t
numbytes_[MAX_VERSION] = {26,44,70,100,134};
static const uint8_t
padbits_[MAX_VERSION] = {0,7,0,0,0};

static const uint8_t
gen1_[8] = {0,87,229,146,149,238,102,21};

static const uint8_t
gen2_[11] = {0,251,67,46,61,118,70,64,94,32,45};

static const uint8_t*
gen_[MAX_VERSION] = {
  (uint8_t*)&gen1_, (uint8_t*)&gen2_, NULL, NULL, NULL
};

struct qrcode_s
{
  uint8_t* stream_;
  uint8_t slen_;
  uint8_t chosen_;
  qrmask_t* masks_[NUM_MASKS];
  uint8_t version_;
};

uint8_t
minversion_(uint8_t count)
{
  uint8_t minv = 0;
  for (; minv < MAX_VERSION; minv++)
  {
    if (count <= strmax_[minv])
    {
      break;
    }
  }
  return minv;
}

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

int
create_qrcode(qrcode_t** self, char* str)
{
  if (*self != NULL)
  {
    return EINVAL;
  }
  size_t str_count = strlen(str);
  if (str_count > strmax_[1])
  {
    // FIXME: set strmax_[0] to latest version added
    fprintf(stderr, "\tstring must be less than %ui characters long\r\n",
            strmax_[1]);
    return EINVAL;
  }
  *self = (qrcode_t*)malloc(sizeof(qrcode_t));
  if (*self == NULL)
  {
    return ENOMEM;
  }
  const uint8_t version = minversion_((uint8_t)str_count);
  (*self)->version_ = version;
  const uint8_t data_len = strmax_[version] + 2;
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
  (*self)->stream_[1] = (uint8_t)str_count << 4;
  uint8_t ui8 = 0;
  for (; ui8 < str_count; ui8++)
  {
    (*self)->stream_[ui8 + 1] |= (uint8_t)(str[ui8] >> 4);
    (*self)->stream_[ui8 + 2] |= (uint8_t)(str[ui8] << 4);
  }

  uint8_t ecc[data_len];
  memcpy(&ecc[0], (*self)->stream_, data_len);
  // NOTE: polynomial division (long division)
  for (ui8 = 0; ui8 < data_len; ui8++)
  {
    uint8_t lead = ecc[0];
    uint8_t uj8 = 0;
    for (; uj8 < ecclen_[version] + 1; uj8++)
    {
      ecc[uj8] ^= alog_[(gen_[version][uj8] + log_[lead]) % GF_MAX];
    }
    if (ecc[0] == 0)
    {
      // NOTE: if multiplier is zero, then discard it
      vshift_(&ecc[0], data_len);
    }
  }
  const uint8_t total_bytes = numbytes_[version];
  uint8_t* tmp_ptr = (uint8_t*)realloc((*self)->stream_, total_bytes);
  if (tmp_ptr == NULL)
  {
    free((*self)->stream_);
    free(*self);
    *self = NULL;
    return ENOMEM;
  }
  (*self)->stream_ = tmp_ptr;
  memcpy(&(*self)->stream_[data_len], &ecc[0], ecclen_[version]);
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
    uint16_t offset = ui8 * 8;
    // NOTE: bitstream goes from bit 7 to bit 0
    int8_t bit = 7;
    for (; bit >= 0; bit--)
    {
      uint8_t module = ((*self)->stream_[ui8] & bitmask_[bit]) >> bit & 1;
      uint8_t uj8 = 0;
      for (; uj8 < NUM_MASKS; uj8++)
      {
        uint16_t index = (uint16_t)(offset + (7 - bit));
        qrmask_set((*self)->masks_[uj8], index, module);
      }
    }
  }
  // NOTE: padding bits
  for (ui8 = 0; ui8 < padbits_[version]; ui8++)
  {
    uint8_t uj8 = 0;
    for (; uj8 < NUM_MASKS; uj8++)
    {
      uint16_t index = (uint16_t)(total_bytes * 8) + ui8;
      qrmask_set((*self)->masks_[uj8], index, 0);
    }
  }
  int32_t curr_score = 0;
  int32_t min_score = UINT16_MAX;
  uint8_t chosen = 0;
  for (ui8 = 0; ui8 < NUM_MASKS; ui8++)
  {
    curr_score = qrmask_penalty((*self)->masks_[ui8]);
    if (curr_score < min_score)
    {
      min_score = curr_score;
      chosen = ui8;
    }
  }
  (*self)->chosen_ = chosen;
  qrmask_apply((*self)->masks_[chosen]);
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

void qrcode_print(qrcode_t* self)
{
  qrmask_print(self->masks_[self->chosen_]);
}
