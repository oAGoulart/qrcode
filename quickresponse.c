#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "heaparray.h"
#include "mask.h"
#include "packedbits.h"
#include "quickresponse.h"
#include "shared.h"

#define GEN_MODE    4
#define NUM_PADBITS 7

extern const uint8_t logt[];
extern const uint8_t alogt[];
extern const uint8_t rsgen[];

typedef enum csubset_e
{
  SUBSET_NUMERIC = 1,
  SUBSET_ALPHA   = 2,
  SUBSET_BYTE    = 4
} csubset_t;

static __inline__ csubset_t __attribute__((__const__))
which_subset_(const uint8_t c)
{
  if (c >= 0x30 && c <= 0x39)
  {
    return SUBSET_NUMERIC;
  }
  if ((c >= 0x41 && c <= 0x5A) || strchr(" $%*+-./:", c) != NULL)
  {
    return SUBSET_ALPHA;
  }
  return SUBSET_BYTE;
}

static __inline__ uint32_t __attribute__((__nonnull__))
count_segment_(const char* __restrict__ str, const csubset_t subset)
{
  uint32_t count = 0;
  size_t i = 0;
  for (; str[i] != '\0'; i++)
  {
    if (which_subset_(str[i]) != subset)
    {
      break;
    }
  }
  return count;
}

static __inline__ uint8_t __attribute__((__const__))
minimum_segment_(const uint8_t version, const uint8_t iteration)
{
  const uint8_t lengths[7][3] = {
    { 6, 7, 8 },
    { 4, 4, 5 },
    { 7, 8, 9 },
    { 13, 15, 17 },
    { 6, 8, 9 },
    { 6, 7, 8 },
    { 11, 15, 16 }
  };
  if (version < 10)
  {
    return lengths[iteration][0];
  }
  else if (version < 27)
  {
    return lengths[iteration][1];
  }
  return lengths[iteration][2];
}

static __inline__ uint8_t __attribute__((__const__, unused))
maximum_count_(const uint8_t version, const csubset_t subset)
{
  const uint8_t lengths[3][3] = {
    { 10, 9, 8 },
    { 12, 11, 16 },
    { 14, 13, 16 }
  };
  if (version < 10)
  {
    return lengths[subset][0];
  }
  else if (version < 27)
  {
    return lengths[subset][1];
  }
  return lengths[subset][2];
}

struct qrcode_s
{
  qrmask_t* masks_[NUM_MASKS];
  pbits_t* bits_;
  uint8_t chosen_;
  uint8_t version_;
};

int
create_qrcode(qrcode_t** self, const char* __restrict__ str, 
              int vnum, bool optimize, bool verbose)
{
  const uint8_t bitmask[CHAR_BIT] = {
    1u, 2u, 4u, 8u, 16u, 32u, 64u, 128u
  };
  const uint8_t cwmax[MAX_VERSION] = {
    17u, 32u, 53u, 78u, 106u
  };
  const uint8_t ecclen[MAX_VERSION] = {
    7u, 10u, 15u, 20u, 26u
  };
  if (*self != NULL)
  {
    eprintf("pointer to garbage in *self");
    return EINVAL;
  }

  /* WARNING WORK IN PROGRESS BELOW */
  uint8_t version = (vnum >= 0 && vnum < MAX_VERSION) ?
    vnum - 1 : MAX_VERSION - 1;
  size_t strcount = strlen(str);
  // NOTE: initial version selection
  uint8_t ui8 = 0;
  if (vnum != version)
  {
    for (; ui8 <= version; ui8++)
    {
      if (strcount <= cwmax[ui8])
      {
        version = ui8;
        break;
      }
    }
  }

  size_t __attribute__((unused)) cwmin = 0;
  bool __attribute__((unused)) encodechanged = true;
  csubset_t __attribute__((unused)) encode = SUBSET_BYTE;
  if (optimize)
  {
    if (which_subset_(str[0]) != SUBSET_BYTE)
    {
      uint32_t next = count_segment_(str, SUBSET_ALPHA);
      if (which_subset_(str[next]) == SUBSET_BYTE &&
          minimum_segment_(version, 0))
      {
        encode = SUBSET_ALPHA;
      }
      else
      {
        next = count_segment_(str, SUBSET_NUMERIC);
        if (which_subset_(str[next]) == SUBSET_BYTE &&
            minimum_segment_(version, 1))
        {
          // encode = SUBSET_BYTE; // redundant;
        }
        else if (which_subset_(str[next]) == SUBSET_ALPHA &&
                 minimum_segment_(version, 2))
        {
          encode = SUBSET_ALPHA;
        }
        else
        {
          encode = SUBSET_NUMERIC;
        }
      }
    }
    size_t i = 0;
    for (; i < strcount; i++)
    {
      switch (encode)
      {
      case SUBSET_NUMERIC:
      {
        // TODO: encode numeric 
        if (encodechanged)
        {
          //Add mode indicator and segment count
        }
        else
        {
          //Append bits to stream
        }
        csubset_t subset = which_subset_(str[i + 1]);
        if (subset != SUBSET_NUMERIC)
        {
          encode = subset;
          encodechanged = true;
        }
        break;
      }
      case SUBSET_ALPHA:
      {
        // TODO: encode alpha
        csubset_t subset = which_subset_(str[i + 1]);
        if (subset == SUBSET_BYTE)
        {
          encode = subset;
          encodechanged = true;
          break;
        }
        uint32_t seg = count_segment_(&str[i], SUBSET_NUMERIC);
        if (which_subset_(str[i]) &&
            seg - i >= minimum_segment_(version, 3))
        {
          encode = SUBSET_NUMERIC;
          encodechanged = true;
        }
        break;
      }
      default:
      {
        // TODO: encode byte
        uint32_t seg = count_segment_(&str[i], SUBSET_NUMERIC);
        csubset_t subset = which_subset_(str[i]);
        if (subset == SUBSET_BYTE &&
            seg - i >= minimum_segment_(version, 4))
        {
          encode = SUBSET_NUMERIC;
          encodechanged = true;
          break;
        }
        if (subset == SUBSET_ALPHA &&
            seg - i >= minimum_segment_(version, 5))
        {
          // NOTE: redundant if MAX_VERSION < 10
          //       keep it for further updates
          encode = SUBSET_NUMERIC;
          encodechanged = true;
          break;
        }
        seg = count_segment_(&str[i], SUBSET_ALPHA);
        if (subset == SUBSET_BYTE &&
            seg - i >= minimum_segment_(version, 6))
        {
          encode = SUBSET_ALPHA;
          encodechanged = true;
        }
        break;
      }
      }
    }
    // NOTE: select min version, with new compact size
    for (ui8 = version; ui8 > 0; ui8--)
    {
      if (cwmin <= cwmax[ui8 - 1])
      {
        continue;
      }
      else
      {
        break;
      }
    }
    if (version == 0)
    {
      eprintf("optimized data length is larger than non-optimized, somehow");
      return ENOTRECOVERABLE;
    }
    version = ui8;
  }
  /* WARNING WORK IN PROGRESS ABOVE */

  if (strcount > cwmax[version])
  {
    eprintf("data must be less than %hhu characters long", cwmax[version]);
    return EINVAL;
  }
  *self = (qrcode_t*)malloc(sizeof(qrcode_t));
  if (*self == NULL)
  {
    eprintf("cannot allocate %zu bytes", sizeof(qrcode_t));
    return ENOMEM;
  }
  (*self)->bits_ = NULL;
  int err = create_pbits(&(*self)->bits_);
  if (err)
  {
    eprintf("cannot create pbits member of qrcode");
    free(*self);
    *self = NULL;
    return err;
  }
  uint16_t offset = 0;
  for (ui8 = 0; ui8 < version; ui8++)
  {
    offset += ecclen[ui8] + 1;
  }
  const uint8_t* gen = rsgen + offset;
  if (verbose)
  {
    pinfo("String length: %zu", strcount);
    pinfo("Version selected: %u", version + 1u);
  }

  (*self)->version_ = version;
  const uint8_t datalen = cwmax[version] + 2;

  pdebug("encoding data bits");
  harray_t* arr = pbits_bytes((*self)->bits_);
  pbits_push((*self)->bits_, GEN_MODE, 4);
  pbits_push((*self)->bits_, strcount, 8);
  for (ui8 = 0; ui8 < strcount; ui8++)
  {
    pbits_push((*self)->bits_, str[ui8], 8);
  }
  // NOTE: padding bytes
  for (ui8 += 2; ui8 < datalen; ui8++)
  {
    pbits_push((*self)->bits_, 0, 8);
  }
  pbits_flush((*self)->bits_);

  const uint8_t eccn = datalen + ecclen[version];
  uint8_t ecc[eccn];
  memset(ecc, 0, eccn);
  harray_copy(arr, ecc, datalen);
  pdebug("starting polynomial division (long division)");
  for (ui8 = 0; ui8 < datalen; ui8++)
  {
    uint8_t lead = ecc[ui8];
    uint8_t uj8 = 0;
    for (; uj8 <= ecclen[version]; uj8++)
    {
      ecc[ui8 + uj8] ^= alogt[(gen[uj8] + logt[lead]) % UINT8_MAX];
    }
  }
  harray_push(arr, &ecc[ui8], ecclen[version]);

  if (verbose)
  {
    pinfo("Calculated codewords (%hhu):", eccn);
    printf("0x%x", harray_byte(arr, 0));
    for (ui8 = 1; ui8 < eccn; ui8++)
    {
      printf(", 0x%x", harray_byte(arr, ui8));
    }
    puts("");
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
  pdebug("applying XOR masks");
  size_t arrlen = harray_length(arr);
  for (ui8 = 0; ui8 < arrlen; ui8++)
  {
    offset = ui8 * 8;
    // NOTE: bitstream goes from bit 7 to bit 0
    int8_t bit = 7;
    for (; bit >= 0; bit--)
    {
      uint8_t module =
        (harray_byte(arr, ui8) & bitmask[bit]) >> bit & 1;
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
        uint16_t index = (uint16_t)(arrlen * 8) + ui8;
        qrmask_set((*self)->masks_[uj8], index, MASK_LIGHT);
      }
    }
  }

  pdebug("calculating masks penalty");
  uint16_t minscore = UINT16_MAX;
  uint8_t chosen = 0;
  for (ui8 = 0; ui8 < NUM_MASKS; ui8++)
  {
    uint16_t score = qrmask_penalty((*self)->masks_[ui8]);
    qrmask_apply((*self)->masks_[ui8]);
    if (score < minscore)
    {
      minscore = score;
      chosen = ui8;
    }
    if (verbose)
    {
      pinfo("Mask [%hhu] penalty: %hu", ui8, score);
    }
  }
  (*self)->chosen_ = chosen;
  if (verbose)
  {
    pinfo("Mask chosen: %hhu", chosen);
  }
  return 0;
}

void
delete_qrcode(qrcode_t** self)
{
  if (*self != NULL)
  {
    if ((*self)->bits_ != NULL)
    {
      delete_pbits(&(*self)->bits_);
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

__inline__ int
qrcode_forcemask(qrcode_t* self, int mask)
{
  if (mask >= 0 && mask < 8)
  {
    self->chosen_ = (uint8_t)mask;
    return 0;
  }
  return EINVAL;
}

__inline__ void
qrcode_print(qrcode_t* self, bool useraw)
{
  if (useraw)
  {
    qrmask_praw(self->masks_[self->chosen_]);
  }
  else
  {
    qrmask_pbox(self->masks_[self->chosen_]);
  }
}

int
qrcode_output(qrcode_t* self, imgfmt_t fmt, int scale,
              const char* __restrict__ filename)
{
  scale = (scale == -1) ? 1 : scale;
  if (scale < 1 || scale > MAX_SCALE)
  {
    eprintf("invalid image scale: %d", scale);
    return EINVAL;
  }
  FILE* f = fopen(filename, "wb+");
  if (f == NULL)
  {
    eprintf("cannot create file: %s", filename);
    return errno;
  }
  int err = 0;
  switch (fmt)
  {
  case FMT_BMP:
  {
    pdebug("bitmap image output selected");
    err = qrmask_outbmp(self->masks_[self->chosen_], scale, f);
    break;
  }
  case FMT_SVG:
  {
    pdebug("vector image output selected");
    qrmask_outsvg(self->masks_[self->chosen_], f);
    break;
  }
  default:
  {
    eprintf("invalid image format selected: %d", fmt);
    err = EINVAL;
    break;
  }
  }
  fclose(f);
  return err;
}
