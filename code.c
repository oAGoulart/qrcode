#include "code.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bytes.h"
#include "vector.h"
#include "data.h"
#include "mask.h"
#include "bits.h"
#include "shared.h"

extern const qrinfo_t qrinfo[];

typedef enum subset_e
{
  SUBSET_NONE,
  SUBSET_NUMERIC,
  SUBSET_ALPHA,
  SUBSET_KANJI,
  SUBSET_BYTE,
  SUBSET_LIMIT
} __attribute__((packed)) subset_t;

static const subset_t subset_lut_[256] = {
  [0 ... 31]    = SUBSET_BYTE,
  [' ']         = SUBSET_ALPHA,
  [33 ... 35]   = SUBSET_BYTE,
  ['$']         = SUBSET_ALPHA,
  ['%']         = SUBSET_ALPHA,
  [38 ... 41]   = SUBSET_BYTE,
  ['*']         = SUBSET_ALPHA,
  ['+']         = SUBSET_ALPHA,
  [44]          = SUBSET_BYTE,
  ['-']         = SUBSET_ALPHA,
  ['.']         = SUBSET_ALPHA,
  ['/']         = SUBSET_ALPHA,
  ['0' ... '9'] = SUBSET_NUMERIC,
  [':']         = SUBSET_ALPHA,
  [59 ... 64]   = SUBSET_BYTE,
  ['A' ... 'Z'] = SUBSET_ALPHA,
  [91 ... 255]  = SUBSET_BYTE
};

static const uint8_t frombyte_lut_[256] = {
  [0 ... 31] = UINT8_MAX,
  [' '] = 36,
  [33 ... 35] = UINT8_MAX,
  ['$'] = 37, ['%'] = 38,
  [38 ... 41] = UINT8_MAX,
  ['*'] = 39, ['+'] = 40,
  [44] = UINT8_MAX,
  ['-'] = 41, ['.'] = 42, ['/'] = 43,
  ['0'] = 0, ['1'] = 1, ['2'] = 2, ['3'] = 3, ['4'] = 4, ['5'] = 5,
  ['6'] = 6, ['7'] = 7, ['8'] = 8, ['9'] = 9, [':'] = 44,
  [59 ... 64] = UINT8_MAX,
  ['A'] = 10, ['B'] = 11, ['C'] = 12, ['D'] = 13, ['E'] = 14,
  ['F'] = 15, ['G'] = 16, ['H'] = 17, ['I'] = 18, ['J'] = 19,
  ['K'] = 20, ['L'] = 21, ['M'] = 22, ['N'] = 23, ['O'] = 24,
  ['P'] = 25, ['Q'] = 26, ['R'] = 27, ['S'] = 28, ['T'] = 29,
  ['U'] = 30, ['V'] = 31, ['W'] = 32, ['X'] = 33, ['Y'] = 34,
  ['Z'] = 35,
  [91 ... 255] = UINT8_MAX
};

typedef struct segment_s
{
  subset_t type;
  size_t   count;
} segment_t;

static __inline__ subset_t __attribute__((__const__))
which_subset_(const uint8_t c)
{
  return subset_lut_[c];
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

typedef enum segphase_e
{
  PHASE_ONE,
  PHASE_TWO,
  PHASE_TREE,
  PHASE_FOUR,
  PHASE_FIVE,
  PHASE_SIX,
  PHASE_SEVEN,
  PHASE_LIMIT
} __attribute__((packed)) segphase_t;

static __inline__ uint8_t __attribute__((__const__))
minimum_segment_(const uint8_t version, const segphase_t phase)
{
  assert(PHASE_LIMIT == 7);
  static const uint8_t lengths[PHASE_LIMIT + 1][3] = {
    { 6, 7, 8 },
    { 4, 4, 5 },
    { 7, 8, 9 },
    { 13, 15, 17 },
    { 6, 8, 9 },
    { 6, 7, 8 },
    { 11, 15, 16 },
    { 0, 0, 0 }
  };
  const uint8_t colidx = (version >= 10) + (version >= 27);
  return lengths[phase][colidx];
}

static __inline__ uint8_t __attribute__((__const__))
maximum_count_(const uint8_t version, const subset_t subset)
{
  assert(SUBSET_LIMIT == 5);
  static const uint8_t lengths[3][SUBSET_LIMIT + 1] = {
    { 0, 10, 9, 0, 8, 0 },
    { 0, 12, 11, 0, 16, 0 },
    { 0, 14, 13, 0, 16, 0 }
  };
  const uint8_t colidx = (version >= 10) + (version >= 27);
  return lengths[colidx][subset];
}

struct qrcode_s
{
  qrmask_t* masks_[NUM_MASKS];
  bits_t*   bits_;
  vector_t* blocks_;
  bytes_t*  modules_;
  uint8_t   selected_mask_;
  uint8_t   version_;
};

static __inline__ uint8_t __attribute__((__const__))
frombyte_(const uint8_t b)
{
  return frombyte_lut_[b];
}

static __inline__ uint8_t __attribute__((__const__))
remainderbits_(const uint8_t version)
{
  if (version > 0 && version < 6)
  {
    return 7;
  }
  else if ((version > 12 && version < 20) || (version > 26 && version < 34))
  {
    return 3;
  }
  else if (version > 19 && version < 27)
  {
    return 4;
  }
  return 0;
}

int
create_qrcode(qrcode_t** self, const char* __restrict__ str,
              const int version, const eclevel_t level,
              const bool optimize, const bool verbose)
{
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
  int err = create_bits(&(*self)->bits_);
  if (err)
  {
    eprintf("cannot create bits_ member of qrcode");
    free(*self);
    *self = NULL;
    return err;
  }
  bytes_t* arr = bits_bytes((*self)->bits_);
  uint8_t ver = (version >= 0 && version < MAX_VERSION) ?
    version - 1 : MAX_VERSION - 1;
  size_t strcount = strlen(str);
  /* NOTE: initial version selection */
  size_t i = 0;
  if (version != ver)
  {
    for (; i <= ver; i++)
    {
      if (strcount <= qrinfo[i * EC_COUNT + level].len - 2)
      {
        ver = i;
        break;
      }
    }
  }
  (*self)->version_ = ver;
  if (verbose)
  {
    pinfo("Queuing subset segments");
  }
  bytes_t* segments = NULL;
  err = create_bytes(&segments, sizeof(segment_t));
  if (err)
  {
    eprintf("cannot create heaparray for subset segments");
    delete_qrcode(self);
    return err;
  }
  segment_t segment = { SUBSET_BYTE, 0 };
  if (optimize)
  {
    pdebug("optimizing data bits");
    segment_t seg = count_segment_(str);
    segment.count = seg.count;
    if (seg.type != SUBSET_BYTE)
    {
      if (seg.type == SUBSET_ALPHA &&
          seg.count >= minimum_segment_(ver, PHASE_ONE))
      {
        segment.type = SUBSET_ALPHA;
      }
      else
      {
        if (seg.count >= minimum_segment_(ver, PHASE_TWO))
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
        if (seg.type == SUBSET_BYTE ||
           (seg.type == SUBSET_NUMERIC &&
            seg.count > minimum_segment_(ver, PHASE_FOUR) &&
            which_subset_(str[i + 1 + seg.count]) == SUBSET_ALPHA))
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
              seg.count >= minimum_segment_(ver, PHASE_FIVE))
          {
            pushseg = true;
          }
          if (subset == SUBSET_ALPHA &&
              seg.count >= minimum_segment_(ver, PHASE_SIX))
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
              seg.count >= minimum_segment_(ver, PHASE_SEVEN))
          {
            pushseg = true;
          }
        }
        break;
      } /* default */
      } /* switch */
      if (pushseg)
      {
        bytes_push(segments, &segment, sizeof(segment_t));
        segment = seg;
        i += seg.count - 1;
        pushseg = false;
      }
      else
      {
        segment.count++;
      }
    }
    bytes_push(segments, &segment, sizeof(segment_t));
  }
  else
  {
    segment.count = strcount;
    bytes_push(segments, &segment, sizeof(segment_t));
  }

  if (verbose)
  {
    pinfo("Encoding data bits");
  }
  const size_t seglen = bytes_length(segments) / sizeof(segment_t);
  size_t k = 0;
  for (i = 0; i < seglen; i++)
  {
    bytes_at(segments, i, sizeof(segment_t), &segment);
    bits_push((*self)->bits_, segment.type, 4);
    const uint8_t max = maximum_count_(
      (*self)->version_,
      segment.type
    );
    bits_push((*self)->bits_, segment.count, max);
    char bseg[4] = { '\0', '\0', '\0', '\0' };
    uint8_t blen = 0;
    for (size_t j = 0; j < segment.count; j++)
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
          bits_push((*self)->bits_, numch, maxb[blen - 1]);
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
            bits_push((*self)->bits_, value, 11);
          }
          else
          {
            uint8_t value = frombyte_(bseg[0]);
            bits_push((*self)->bits_, value, 6);
          }
          blen = 0;
        }
        break;
      }
      default:
      {
        bits_push((*self)->bits_, str[k + j], 8);
        break;
      } /* default */
      } /* switch */
    }
    k += segment.count;
  }
  bits_flush((*self)->bits_);
  delete_bytes(&segments);

  /* NOTE: final version selection */
  size_t datalen = bytes_length(arr);
  for (i = ver; i > 0; i--)
  {
    if (datalen > qrinfo[(i - 1) * EC_COUNT + level].len)
    {
      break;
    }
  }
  ver = i;
  (*self)->version_ = ver;
  const qrinfo_t* finalvl = &qrinfo[ver * EC_COUNT + level];
  if (datalen > finalvl->len)
  {
    eprintf("data must be less than %u characters long", finalvl->len - 2u);
    delete_qrcode(self);
    return EINVAL;
  }
  if (verbose)
  {
    pinfo("String length: %zu", strcount);
    pinfo("Data length: %zu", datalen - 2u);
    pinfo("Version selected: %u", ver + 1u);
  }
  /* NOTE: padding bytes */
  const size_t padbytes = finalvl->len - datalen;
  for (i = 0; i < padbytes; i++)
  {
    uint8_t pad_byte = (i % 2 == 0) ? 0xec : 0x11;
    bits_push((*self)->bits_, pad_byte, 8);
  }

  if (verbose)
  {
    pinfo("Encoded codewords (%hu):", finalvl->len);
    printf("0x%x", bytes_byte(arr, 0));
    for (i = 1; i < finalvl->len; i++)
    {
      printf(", 0x%x", bytes_byte(arr, i));
    }
    puts("");
  }

  (*self)->blocks_ = NULL;
  err = create_vector(&(*self)->blocks_,
    (void (*)(void **))&delete_qrdata);
  if (err != 0)
  {
    eprintf("could not create vector type");
    delete_qrcode(self);
    return err;
  }

  pdebug("starting polynomial division (long division)");
  size_t mod = 0;
  uint16_t nblocks = finalvl->blocks[0] + finalvl->blocks[1];
  for (i = 0; i < nblocks; i++)
  {
    uint8_t dlen = (i < finalvl->blocks[0]) ?
      finalvl->datapb[0] : finalvl->datapb[1];
    qrdata_t* qrdata = NULL;
    err = create_qrdata(&qrdata,
      bytes_span(arr, mod),
      dlen, finalvl->eccpb
    );
    if (err != 0)
    {
      eprintf("could not create qrdata type");
      delete_qrcode(self);
      return err;
    }
    vector_push((*self)->blocks_, qrdata);
    mod += dlen;
  }

  size_t fullen = nblocks * finalvl->eccpb + finalvl->len;
  (*self)->modules_ = NULL;
  err = create_bytes(&(*self)->modules_, fullen);
  if (err != 0)
  {
    eprintf("could not create bytes type");
    delete_qrcode(self);
    return err;
  }
  /* NOTE: data interleaving */
  uint8_t highpb = (finalvl->blocks[1] == 0) ?
    finalvl->datapb[0] : finalvl->datapb[1];
  for (i = 0; i < highpb; i++)
  {
    size_t j = 0;
    for (qrdata_t** d = (qrdata_t**)vector_begin((*self)->blocks_);
         d != (qrdata_t**)vector_end((*self)->blocks_); d++, j++)
    {
      if (j < finalvl->blocks[0] && i >= finalvl->datapb[0])
      {
        continue;
      }
      bytes_push((*self)->modules_,
        &qrdata_codewords(*d)[i], 1);
    }
  }
  /* NOTE: ecc interleaving */
  for (i = 0; i < finalvl->eccpb; i++)
  {
    size_t j = 0;
    for (qrdata_t** d = (qrdata_t**)vector_begin((*self)->blocks_);
        d != (qrdata_t**)vector_end((*self)->blocks_); d++, j++)
    {
      uint8_t dlen = (j < finalvl->blocks[0]) ?
        finalvl->datapb[0] : finalvl->datapb[1];
      bytes_push((*self)->modules_,
        &qrdata_codewords(*d)[i + dlen], 1);
    }
  }

  datalen = bytes_length((*self)->modules_);
  if (verbose)
  {
    pinfo("Interleaved codewords (%zu):", datalen);
    printf("0x%x", bytes_byte((*self)->modules_, 0));
    for (i = 1; i < datalen; i++)
    {
      printf(", 0x%x", bytes_byte((*self)->modules_, i));
    }
    puts("");
  }

  pdebug("applying XOR masks");
  memset((*self)->masks_, 0, sizeof((*self)->masks_));
  for (i = 0; i < NUM_MASKS; i++)
  {
    err = create_qrmask(&(*self)->masks_[i],
      ver, level, i);
    if (err)
    {
      delete_qrcode(self);
      return err;
    }
  }
  for (i = 0; i < datalen; i++)
  {
    size_t offset = i * 8;
    /* NOTE: bitstream goes from bit 7 to bit 0 */
    for (int8_t bit = 7; bit >= 0; bit--)
    {
      uint8_t module =
        (bytes_byte((*self)->modules_, i) & 1 << bit) >> bit & 1;
      uint8_t uj8 = 0;
      for (; uj8 < NUM_MASKS; uj8++)
      {
        uint16_t index = (uint16_t)(offset + (7 - bit));
        qrmask_set((*self)->masks_[uj8], index, module);
      }
    }
  }
  /* NOTE: padding bits, MUST check xor */
  for (i = 0; i < remainderbits_(ver); i++)
  {
    for (uint8_t j = 0; j < NUM_MASKS; j++)
    {
      uint16_t index = (uint16_t)(datalen * 8) + i;
      qrmask_set((*self)->masks_[j], index, MASK_LIGHT);
    }
  }

  pdebug("calculating masks penalty");
  uint16_t minscore = UINT16_MAX;
  uint8_t selected = 0;
  for (i = 0; i < NUM_MASKS; i++)
  {
    qrpenalty_t p = qrmask_penalty((*self)->masks_[i]);
    uint16_t score = p.run + p.box + p.finder + p.balance;
    if (score < minscore)
    {
      minscore = score;
      selected = i;
    }
    if (verbose)
    {
      pinfo("Mask [%zu] total penalty: %u\n"
        "         run:     %hu\n"
        "         box:     %hu\n"
        "         finder:  %hu\n"
        "         balance: %hu", i, score,
        p.run, p.box, p.finder, p.balance);
    }
  }
  (*self)->selected_mask_ = selected;
  if (verbose)
  {
    pinfo("Mask selected: %hhu", selected);
  }
  return 0;
}

void
delete_qrcode(qrcode_t** self)
{
  if (*self != NULL)
  {
    delete_bits(&(*self)->bits_);
    delete_vector(&(*self)->blocks_);
    delete_bytes(&(*self)->modules_);
    uint8_t i = 0;
    for (; i < NUM_MASKS; i++)
    {
      delete_qrmask(&(*self)->masks_[i]);
    }
    free(*self);
    *self = NULL;
  }
}

__inline__ uint8_t
qrcode_version(const qrcode_t *self)
{
  return self->version_;
}

__inline__ int
qrcode_forcemask(qrcode_t* self, int mask)
{
  if (mask >= 0 && mask < 8)
  {
    self->selected_mask_ = (uint8_t)mask;
    return 0;
  }
  return EINVAL;
}

__inline__ void
qrcode_print(const qrcode_t* self, bool useraw)
{
  if (useraw)
  {
    qrmask_praw(self->masks_[self->selected_mask_]);
  }
  else
  {
    qrmask_pbox(self->masks_[self->selected_mask_]);
  }
}

int
qrcode_output(const qrcode_t* self, imgfmt_t fmt, int scale,
              const char* __restrict__ filename)
{
  scale = (scale == -1) ? 1 : scale;
  if (scale < 1 || scale > MAX_SCALE)
  {
    eprintf("invalid image scale: %d", scale);
    return EINVAL;
  }
  int err = EIO;
  FILE* f = fopen(filename, "wb+");
  if (f == NULL)
  {
    eprintf("cannot create file: %s", filename);
    return err;
  }
  switch (fmt)
  {
  case FMT_BMP:
  {
    pdebug("bitmap image output selected");
    err = qrmask_outbmp(self->masks_[self->selected_mask_],
                        scale, f);
    break;
  }
  case FMT_SVG:
  {
    pdebug("vector image output selected");
    qrmask_outsvg(self->masks_[self->selected_mask_], f);
    err = 0;
    break;
  }
  default:
  {
    eprintf("invalid image format selected: %d", fmt);
    err = EINVAL;
    break;
  } /* default */
  } /* switch */
  fclose(f);
  return err;
}
