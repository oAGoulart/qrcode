#include <limits.h>
#include <stdbool.h>
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

#define NUM_PADBITS 7

extern const uint8_t logt[];
extern const uint8_t alogt[];
extern const uint8_t rsgen[];

typedef enum subset_e
{
  SUBSET_NUMERIC = 1,
  SUBSET_ALPHA   = 2,
  SUBSET_BYTE    = 4
} __attribute__((packed)) subset_t;

typedef struct segment_s
{
  subset_t type;
  size_t count;
} segment_t;

static __inline__ subset_t __attribute__((__const__))
which_subset_(const uint8_t c)
{
  if (c >= 0x30 && c <= 0x39)
  {
    return SUBSET_NUMERIC;
  }
  if ((c >= 0x41 && c <= 0x5A) || __builtin_strchr(" $%*+-./:", c) != NULL)
  {
    return SUBSET_ALPHA;
  }
  return SUBSET_BYTE;
}

static __inline__ segment_t __attribute__((__nonnull__))
count_segment_(const char* str)
{
  const subset_t subset = which_subset_(str[0]);
  segment_t seg = { subset, 0 };
  size_t i = 0;
  for (; str[i] != '\0'; i++)
  {
    if (which_subset_(str[i]) != subset)
    {
      break;
    }
  }
  seg.count = i;
  return seg;
}

static __inline__ uint8_t __attribute__((__const__))
minimum_segment_(const uint8_t version, const uint8_t iteration)
{
  static const uint8_t lengths[7][3] = {
    { 6, 7, 8 },
    { 4, 4, 5 },
    { 7, 8, 9 },
    { 13, 15, 17 },
    { 6, 8, 9 },
    { 6, 7, 8 },
    { 11, 15, 16 }
  };
  uint8_t colidx = 0;
  if (version >= 10 && version < 27)
  {
    colidx = 1;
  }
  else if (version >= 27)
  {
    colidx = 2;
  }
  return lengths[iteration][colidx];
}

static __inline__ uint8_t __attribute__((__const__))
maximum_count_(const uint8_t version, subset_t subset)
{
  static const uint8_t lengths[3][3] = {
    { 10, 9, 8 },
    { 12, 11, 16 },
    { 14, 13, 16 }
  };
  static const uint8_t subidx[5] = { 0, 0, 1, 0, 2 };
  uint8_t colidx = 0;
  if (version >= 10 && version < 27)
  {
    colidx = 1;
  }
  else if (version >= 27)
  {
    colidx = 2;
  }
  return lengths[colidx][subidx[subset]];
}

struct qrcode_s
{
  qrmask_t* masks_[NUM_MASKS];
  pbits_t* bits_;
  uint8_t chosen_;
  uint8_t version_;
};

static __inline__ uint8_t
frombyte_(uint8_t b)
{
  switch (b)
  {
  case ' ':
    return 36;
  case '$':
    return 37;
  case '%':
    return 38;
  case '*':
    return 39;
  case '+':
    return 40;
  case '-':
    return 41;
  case '.':
    return 42;
  case '/':
    return 43;
  case ':':
    return 44;
  default:
  {
    if (b >= 48 && b <= 57)
    {
      return b - 48;
    }
    else if (b >= 65 && b <= 90)
    {
      return (b - 65) + 10;
    }
  } // default
  } // switch
  return UINT8_MAX;
}

int
create_qrcode(qrcode_t** self, const char* __restrict__ str, 
              int vnum, bool optimize, bool verbose)
{
  static const uint8_t cwmax[MAX_VERSION] = {
    17u, 32u, 53u, 78u, 106u
  };
  static const uint8_t ecclen[MAX_VERSION] = {
    7u, 10u, 15u, 20u, 26u
  };
  if (*self != NULL)
  {
    eprintf("pointer to garbage in *self");
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
  harray_t* arr = pbits_bytes((*self)->bits_);

  uint8_t version = (vnum >= 0 && vnum < MAX_VERSION) ?
    vnum - 1 : MAX_VERSION - 1;
  size_t strcount = __builtin_strlen(str);
  // NOTE: initial version selection
  size_t i = 0;
  if (vnum != version)
  {
    for (; i <= version; i++)
    {
      if (strcount <= cwmax[i])
      {
        version = i;
        break;
      }
    }
  }
  (*self)->version_ = version;

  pinfo("Queuing subset segments");
  harray_t* segments = NULL;
  create_harray(&segments, sizeof(segment_t));
  segment_t segment = { SUBSET_BYTE, 0 };
  if (optimize)
  {
    pdebug("optimizing data bits");
    segment_t seg = count_segment_(str);
    segment.count = seg.count;
    if (seg.type != SUBSET_BYTE)
    {
      if (seg.type == SUBSET_ALPHA &&
          seg.count >= minimum_segment_(version, 0))
      {
        segment.type = SUBSET_ALPHA;
      }
      else
      {
        if (seg.count >= minimum_segment_(version, 1))
        {
          segment.type = SUBSET_NUMERIC;
        }
        else
        {
          segment.type = SUBSET_ALPHA;
        }
      }
    }
    bool pushseg = false;
    for (i = segment.count; i < strcount; i++)
    {
      seg = count_segment_(&str[i]);
      if (seg.type == segment.type)
      {
        i += seg.count - 1;
        segment.count += seg.count;
        continue;
      }
      switch (segment.type)
      {
      case SUBSET_NUMERIC:
      {
        if (seg.type != SUBSET_NUMERIC)
        {
          pushseg = true;
        }
        break;
      }
      case SUBSET_ALPHA:
      {
        if (seg.type == SUBSET_BYTE)
        {
          pushseg = true;
        }
        else if (seg.type == SUBSET_NUMERIC &&
                 seg.count > minimum_segment_(version, 3) &&
                 which_subset_(str[i + 1 + seg.count]) == SUBSET_ALPHA)
        {
          pushseg = true;
        }
        break;
      }
      default:
      {
        if (seg.type == SUBSET_NUMERIC)
        {
          subset_t subset = which_subset_(str[i + 1 + seg.count]);
          if (subset == SUBSET_BYTE &&
              seg.count >= minimum_segment_(version, 4))
          {
            pushseg = true;
          }
          if (subset == SUBSET_ALPHA &&
              seg.count >= minimum_segment_(version, 5))
          {
            pushseg = true;
          }
        }
        else if (seg.type == SUBSET_ALPHA)
        {
          if (seg.count == 0)
          {
            break;
          }
          subset_t subset = which_subset_(str[i + 1 + seg.count]);
          if (subset == SUBSET_BYTE &&
              seg.count >= minimum_segment_(version, 6))
          {
            pushseg = true;
          }
        }
        break;
      } // default
      } // switch
      if (pushseg)
      {
        harray_push(segments, &segment, sizeof(segment_t));
        segment = seg;
        i += seg.count - 1;
        pushseg = false;
      }
      else
      {
        segment.count++;
      }
    }
    harray_push(segments, &segment, sizeof(segment_t));
  }
  else
  {
    segment.count = strcount;
    harray_push(segments, &segment, sizeof(segment_t));
  }

  pinfo("Encoding data bits");
  const size_t seglen = harray_length(segments) / sizeof(segment_t);
  size_t k = 0;
  for (i = 0; i < seglen; i++)
  {
    harray_at(segments, i, sizeof(segment_t), &segment);
    pbits_push((*self)->bits_, segment.type, 4);
    const uint8_t max = maximum_count_((*self)->version_,
                                       segment.type);
    pbits_push((*self)->bits_, segment.count, max);
    char bseg[4] = { '\0', '\0', '\0', '\0' };
    uint8_t blen = 0;
    size_t j = 0;
    for (; j < segment.count; j++)
    {
      switch (segment.type)
      {
      case SUBSET_NUMERIC:
      {
        bseg[blen] = str[k + j];
        blen++;
        if (blen == 3 || j == segment.count - 1)
        {
          const uint8_t maxb[3] = { 4, 7, 10 };
          bseg[blen] = '\0';
          int numch = atoi(bseg);
          pbits_push((*self)->bits_, numch, maxb[blen - 1]);
          blen = 0;
        }
        break;
      }
      case SUBSET_ALPHA:
      {
        bseg[blen] = str[k + j];
        blen++;
        if (blen == 2 || j == segment.count - 1)
        {
          if (blen == 2)
          {
            uint16_t value = (uint16_t)frombyte_(bseg[0]) * 45 +
                             (uint16_t)frombyte_(bseg[1]);
            pbits_push((*self)->bits_, value, 11);
          }
          else
          {
            uint8_t value = frombyte_(bseg[0]);
            pbits_push((*self)->bits_, value, 6);
          }
          blen = 0;
        }
        break;
      }
      default:
      {
        pbits_push((*self)->bits_, str[k + j], 8);
        break;
      } // default
      } // switch
    }
    k += segment.count;
  }
  pbits_flush((*self)->bits_);
  delete_harray(&segments);

  size_t datalen = harray_length(arr);
  for (i = version; i > 0; i--)
  {
    if (datalen <= cwmax[i - 1])
    {
      continue;
    }
    break;
  }
  version = i;
  (*self)->version_ = version;
  if (datalen > cwmax[version] + 2)
  {
    eprintf("data must be less than %hhu characters long", cwmax[version]);
    delete_qrcode(self);
    return EINVAL;
  }
  for (i = 0; i < (cwmax[version] + 2) - datalen; i++)
  {
    // NOTE: padding bytes
    // TODO: change to 64bits
    pbits_push((*self)->bits_, 0, 8);
  }
  datalen = harray_length(arr);
  uint16_t offset = 0;
  for (i = 0; i < version; i++)
  {
    offset += ecclen[i] + 1;
  }
  const uint8_t* gen = rsgen + offset;
  if (verbose)
  {
    pinfo("String length: %zu", strcount);
    pinfo("Version selected: %u", version + 1u);
  }

  pdebug("starting polynomial division (long division)");
  const size_t eccn = datalen + ecclen[version];
  uint8_t ecc[eccn];
  __builtin_memset(ecc + datalen, 0, ecclen[version]);
  harray_copy(arr, ecc, datalen);
  for (i = 0; i < datalen; i++)
  {
    uint8_t lead = ecc[i];
    uint8_t j = 0;
    for (; j <= ecclen[version]; j++)
    {
      ecc[i + j] ^= alogt[(gen[j] + logt[lead]) % UINT8_MAX];
    }
  }
  harray_push(arr, &ecc[i], ecclen[version]);
  datalen = harray_length(arr);

  if (verbose)
  {
    pinfo("Calculated codewords (%zu):", datalen);
    printf("0x%x", harray_byte(arr, 0));
    for (i = 1; i < datalen; i++)
    {
      printf(", 0x%x", harray_byte(arr, i));
    }
    puts("");
  }

  pdebug("applying XOR masks");
  __builtin_memset((*self)->masks_, 0, sizeof((*self)->masks_));
  for (i = 0; i < NUM_MASKS; i++)
  {
    err = create_qrmask(&(*self)->masks_[i], version, i);
    if (err)
    {
      delete_qrcode(self);
      return err;
    }
  }
  for (i = 0; i < datalen; i++)
  {
    offset = i * 8;
    // NOTE: bitstream goes from bit 7 to bit 0
    int8_t bit = 7;
    for (; bit >= 0; bit--)
    {
      uint8_t module =
        (harray_byte(arr, i) & 1 << bit) >> bit & 1;
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
    for (i = 0; i < NUM_PADBITS; i++)
    {
      uint8_t uj8 = 0;
      for (; uj8 < NUM_MASKS; uj8++)
      {
        uint16_t index = (uint16_t)(datalen * 8) + i;
        qrmask_set((*self)->masks_[uj8], index, MASK_LIGHT);
      }
    }
  }

  pdebug("calculating masks penalty");
  uint16_t minscore = UINT16_MAX;
  uint8_t chosen = 0;
  for (i = 0; i < NUM_MASKS; i++)
  {
    uint16_t score = qrmask_penalty((*self)->masks_[i]);
    qrmask_apply((*self)->masks_[i]);
    if (score < minscore)
    {
      minscore = score;
      chosen = i;
    }
    if (verbose)
    {
      pinfo("Mask [%zu] penalty: %u", i, score);
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
    delete_pbits(&(*self)->bits_);
    uint8_t i = 0;
    for (; i < NUM_MASKS; i++)
    {
      delete_qrmask(&(*self)->masks_[i]);
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
