#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mask.h"
#include "shared.h"

#define MASKINFO_LEN 15

extern const uint16_t qrindex[];

struct qrmask_s
{
  const uint16_t* i_;
  uint16_t count_;
  uint8_t* v_ __attribute__((nonstring));
  uint8_t  version_;
  uint8_t  order_;
  uint8_t  masknum_;
  uint16_t dark_;
  uint16_t light_;
  uint16_t penalty_;
};

static __inline__ bool __attribute__((const))
should_xor_(const uint8_t order, const uint16_t index, const uint8_t pattern)
{
  const uint16_t row = (uint16_t)floor(index / order);
  const uint16_t col = index % order;
  switch (pattern)
  {
  case 0:
    return (row + col) % 2 == 0;
  case 1:
    return row % 2 == 0;
  case 2:
    return col % 3 == 0;
  case 3:
    return (row + col) % 3 == 0;
  case 4:
    return (long)(floor(row / 2) + floor(col / 3)) % 2 == 0;
  case 5:
    return ((row * col) % 2) + ((row * col) % 3) == 0;
  case 6:
    return (((row * col) % 2) + ((row * col) % 3)) % 2 == 0;
  case 7:
    return (((row + col) % 2) + ((row * col) % 3)) % 2 == 0;
  default:
    return false;
  }
}

static __inline__ int __attribute__((__nonnull__))
colcmp_(const uint8_t* __restrict__ v, const uint8_t order,
        const uint16_t n, const uint8_t arr[n])
{
  uint16_t i = 0;
  for (; i < n; i++)
  {
    int diff = v[i * order] - arr[i];
    if (diff != 0)
    {
      return diff;
    }
  }
  return 0;
}

static void __attribute__((__nonnull__))
mask_double_(const uint8_t* __restrict__ v, uint8_t order)
{
  char str[(order * sizeof(uint32_t)) + 1];
  uint8_t top = 0;
  uint8_t bottom = 0;
  uint16_t i = 0;
  for (bottom = order; top < order; top++, bottom++)
  {
    uint8_t ch_case = (uint8_t)(v[top] << 1) + v[bottom];
    switch (ch_case)
    {
    case 0:
      strcpy(&str[i], " ");
      i += 3;
      break;
    case 1:
      strcpy(&str[i], "▄");
      i += 3;
      break;
    case 2:
      strcpy(&str[i], "▀");
      i += 3;
      break;
    default:
      strcpy(&str[i], "█");
      i += 3;
      break;
    }
  }
  if (i > 0)
  {
    str[i + 1] = '\0';
    printf("    %s    " __nl, str);
  }
}

static void __attribute__((__nonnull__))
mask_single_(const uint8_t* __restrict__ v, uint8_t order)
{
  char str[(order * sizeof(uint32_t)) + 1];
  uint8_t top = 0;
  uint16_t i = 0;
  for (; top < order; top++)
  {
    if (v[top] == 0)
    {
      strcpy(&str[i], " ");
      i += 3;
    }
    else
    {
      strcpy(&str[i], "▀");
      i += 3;
    }
  }
  if (i > 0)
  {
    str[i + 1] = '\0';
    printf("    %s    " __nl, str);
  }
}

static void __attribute__((__nonnull__))
place_finder_(qrmask_t* self)
{
  const uint8_t finder[7][7] = {
    { 1, 1, 1, 1, 1, 1, 1 },
    { 1, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 1, 1, 1, 0, 1 },
    { 1, 0, 1, 1, 1, 0, 1 },
    { 1, 0, 1, 1, 1, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 1 },
    { 1, 1, 1, 1, 1, 1, 1 },
  };
  size_t i = 0;
  for (; i < 7; i++)
  {
    memcpy(&self->v_[self->order_ * i], &finder[i], 7u);
    memcpy(&self->v_[(self->order_ - 7u) + self->order_ * i], &finder[i], 7u);
    memcpy(&self->v_[(self->order_ - 7u) * self->order_ +
                     self->order_ * i], &finder[i], 7u);
  }
  // NOTE: separators not required (array is initialized to 0)
}

static void __attribute__((__nonnull__))
place_align_(qrmask_t* self)
{
  const uint8_t align[5][5] = {
    { 1, 1, 1, 1, 1 },
    { 1, 0, 0, 0, 1 },
    { 1, 0, 1, 0, 1 },
    { 1, 0, 0, 0, 1 },
    { 1, 1, 1, 1, 1 }
  };
  size_t index = (self->order_ - 9u) * self->order_ + self->order_ - 9u;
  size_t i = 0;
  for (; i < 5; i++)
  {
    memcpy(&self->v_[index + i * self->order_], &align[i], 5u);
  }
}

static void __attribute__((__nonnull__))
place_timing_(qrmask_t* self)
{
  size_t idx1 = self->order_ * 6u + 8u;
  size_t idx2 = self->order_ * 8u + 6u;
  size_t i = 0;
  for (; i < self->order_ - 16u; i++)
  {
    self->v_[idx1] = (i % 2 == 0) ? MASK_DARK : MASK_LIGHT;
    self->v_[idx2] = (i % 2 == 0) ? MASK_DARK : MASK_LIGHT;
    idx1++;
    idx2 += self->order_;
  }
}

static void __attribute__((__nonnull__))
percentage_penalty_(qrmask_t* self)
{
  double percentage = (self->dark_ / self->count_) * 10;
  double prev = floor(percentage) * 10;
  double next = percentage - fmod(percentage * 10, 5.0) + 5;
  prev = fabs(prev - 50) / 5;
  next = fabs(next - 50) / 5;
  self->penalty_ += (uint16_t)(fmin(prev, next) * 10);
}

static void __attribute__((__nonnull__))
module_penalty_(qrmask_t* self)
{
  const uint8_t patleft[9] = { 1, 1, 1, 0, 1, 0, 0, 0, 0 };
  const uint8_t patright[9] = { 1, 1, 1, 0, 1, 0, 0, 0, 0 };
  uint8_t i = 0;
  for (; i < self->order_; i++)
  {
    uint16_t row = i * self->order_;
    uint8_t j = 0;
    // Step 1: row direction >>>
    for (; j < self->order_ - 1; j++)
    {
      uint8_t* module = &self->v_[row + j];
      uint8_t* next = &self->v_[row + j + 1];
      if (*module == *next)
      {
        // NOTE: square penalty
        if (i < self->order_ - 1) {
          if (*(module + self->order_) == *module &&
              *(next + self->order_) == *module)
          {
            self->penalty_ += 3;
          }
        }
        // NOTE: sequential line penalty (row)
        if (j < self->order_ - 4)
        {
          uint8_t count = j;
          for (; next + j + 1 < self->v_ + self->count_; j++)
          {
            if (*(next + j + 1) != *module)
            {
              break;
            }
          }
          count = j - count;
          if (count > 4)
          {
            self->penalty_ += (count - 5) + 3;
          }
        }
      }
      // NOTE: pattern penalty (row)
      if (j < self->order_ - 10 && *next == MASK_LIGHT)
      {
        const uint8_t* pattern = (*module == MASK_DARK) ? patleft : patright;
        if (!memcmp(module + 2, pattern, 9))
        {
          self->penalty_ += 40;
        }
      }
    }
    // Step 2: column direction vvv
    uint16_t k = 0;
    for (; k < self->count_ - (4 * self->order_); k += self->order_)
    {
      uint8_t* module = &self->v_[i + k];
      uint8_t* next = &self->v_[i + k + self->order_];
      if (*module == *next)
      {
        // NOTE: sequential line penalty (column)
        uint16_t count = k;
        for (; next + k < self->v_ + self->count_; k += self->order_)
        {
          if (*(next + k) != *module)
          {
            break;
          }
        }
        count = (uint16_t)floor((k - count) / self->order_);
        if (count > 4)
        {
          self->penalty_ += (count - 5) + 3;
        }
      }
      // NOTE: pattern penalty (column)
      if (k < self->count_ - (10 * self->order_) && *next == MASK_LIGHT)
      {
        const uint8_t* pattern = (*module == MASK_DARK) ? patleft : patright;
        if (!colcmp_(module + (2 * self->order_), self->order_, 9, pattern))
        {
          self->penalty_ += 40;
        }
      }
    }
  }
}

int
create_qrmask(qrmask_t** self, uint8_t version, uint8_t masknum)
{
  const uint8_t qr_order[MAX_VERSION] = { 21u, 25u, 29u, 33u, 37u };
  const uint16_t qr_count[MAX_VERSION] = { 441u, 625u, 841u, 1089u, 1369u };
  const uint16_t qr_basedark[MAX_VERSION] = { 91u, 112u, 114u, 118u, 122u };
  const uint16_t qr_baselight[MAX_VERSION] = { 127u, 139u, 141u, 145u, 149u };
  const uint16_t qr_offset[MAX_VERSION] = { 0, 208u, 567u, 1134u, 1941u };

  if (*self != NULL)
  {
    eprintf("pointer to garbage in *self");
    return EINVAL;
  }
  if (version >= MAX_VERSION)
  {
    eprintf("invalid qrcode version: %u", version);
    return EINVAL;
  }
  *self = (qrmask_t*)malloc(sizeof(qrmask_t));
  if (*self == NULL)
  {
    eprintf("cannot allocate %u bytes", (uint32_t)sizeof(qrmask_t));
    return ENOMEM;
  }
  (*self)->version_ = version;
  (*self)->order_ = qr_order[version];
  (*self)->count_ = qr_count[version];
  (*self)->masknum_ = masknum;
  (*self)->i_ = qrindex + qr_offset[version];
  (*self)->v_ = (uint8_t*)malloc((*self)->count_);
  if ((*self)->v_ == NULL)
  {
    eprintf("cannot allocate %u bytes", (*self)->count_);
    free(*self);
    *self = NULL;
    return ENOMEM;
  }
  memset((*self)->v_, 0, (*self)->count_);
  (*self)->dark_ = qr_basedark[version];
  (*self)->light_ = qr_baselight[version];
  (*self)->penalty_ = 0;
  place_finder_(*self);
  place_timing_(*self);
  if (version > 0)
  {
    place_align_(*self);
  }
  // NOTE: dark module
  (*self)->v_[((*self)->order_ - 8) * (*self)->order_ + 8] = MASK_DARK;
  return 0;
}

void
delete_qrmask(qrmask_t** self)
{
  if (*self != NULL)
  {
    if ((*self)->v_ != NULL)
    {
      free((*self)->v_);
    }
    free(*self);
    *self = NULL;
  }
}

void
qrmask_set(qrmask_t* self, uint16_t index, uint8_t module)
{
  const uint16_t idx = self->i_[index];
  if (should_xor_(self->order_, idx, self->masknum_))
  {
    module = (module == MASK_DARK) ? MASK_LIGHT : MASK_DARK;
  }
  self->v_[idx] = module;
  if (module == MASK_LIGHT)
  {
    self->light_++;
  }
  else
  {
    self->dark_++;
  }
}

uint16_t
qrmask_penalty(qrmask_t* self)
{
  if (self->penalty_ == 0)
  {
    percentage_penalty_(self);
    module_penalty_(self);
  }
  return self->penalty_;
}

void
qrmask_apply(qrmask_t* self)
{
  const uint16_t maskinfo[NUM_MASKS] = {
    30660u, 29427u, 32170u, 30877u,
    26159u, 25368u, 27713u, 26998u
  };
  uint8_t i = 0;
  for (; i < MASKINFO_LEN; i++)
  {
    int idx1 = 0;
    int idx2 = 0;
    if (i < 8)
    {
      idx1 = self->order_ * 8 + i;
      idx1 += (i > 5) ? 1 : 0;
    }
    else
    {
      idx1 = (MASKINFO_LEN - i) * self->order_ + 8;
      idx1 -= (i > 8) ? self->order_ : 0;
    }
    if (i < 7)
    {
      idx2 = self->order_ * (self->order_ - i - 1) + 8;
    }
    else
    {
      idx2 = self->order_ * 8 + self->order_ - 8 + i - 7;
    }
    self->v_[idx1] = (maskinfo[self->masknum_] >> (MASKINFO_LEN - i - 1)) & 1;
    self->v_[idx2] = (maskinfo[self->masknum_] >> (MASKINFO_LEN - i - 1)) & 1;
  }
}

void
qrmask_pbox(qrmask_t* self)
{
  puts(__nl);
  uint16_t line = 0;
  for (; line < self->order_ - 1; line += 2)
  {
    mask_double_(&self->v_[line * self->order_], self->order_);
  }
  if (self->order_ % 2 != 0)
  {
    mask_single_(&self->v_[(self->order_ - 1) * self->order_], self->order_);
  }
  puts(__nl);
}

void
qrmask_praw(qrmask_t* self)
{
  size_t row = 0;
  for (; row < self->order_; row++)
  {
    printf("%u", self->v_[row * self->order_]);
    size_t col = 1;
    for (; col < self->order_; col++)
    {
      printf(", %u", self->v_[row * self->order_ + col]);
    }
    puts("");
  }
}

typedef struct __attribute__((packed)) bitmap_s
{
  char signature[2];
  uint32_t filesize;
  uint32_t reserved;
  uint32_t dataoffset;
  uint32_t infosize;
  uint32_t width;
  uint32_t height;
  uint16_t planes;
  uint16_t bitcount;
  uint32_t compression;
  uint32_t imagesize;
  uint32_t xppm;
  uint32_t yppm;
  uint32_t colorsused;
  uint32_t colorsimportant;
  char colortable[2][4];
} bitmap_t;

int
qrmask_outbmp(qrmask_t* self, uint8_t scale, FILE* __restrict__ file)
{
  const uint32_t nbits = (8 + self->order_) * scale;
  const uint8_t nlongs = (uint8_t)ceil((double)nbits / 32);
  const uint8_t nbytes = nlongs * 4;
  const uint32_t datalen = nlongs * nbits;
  const uint32_t offset = (uint32_t)sizeof(bitmap_t);
  bitmap_t bm = {
    { 'B', 'M' }, offset + datalen, 0, offset, 40, nbits, nbits, 1, 1, 0,
    datalen, 4800, 4800, 2, 0, { { -1, -1, -1, 0 }, { 0, 0, 0, 0 } }
  };
  pdebug("writing bitmap header");
  if (!fwrite(&bm, sizeof(bitmap_t), 1, file))
  {
    eprintf("corrupted bitmap format");
    return EIO;
  }
  pdebug("writing bitmap raster data");
  int16_t i = 0;
  for (; i < nbytes * 4 * scale; i++)
  {
    fputc('\0', file);
  }
  for (i = self->order_ - 1; i >= 0; i--)
  {
    uint8_t* module = self->v_ + i * self->order_;
    uint8_t bytes[nbytes];
    uint16_t mcount = 0;
    int16_t scount = scale;
    uint8_t rest = (4 * scale) % 8;
    uint16_t index = (uint16_t)(4 * scale - rest) / 8;
    uint16_t j = 0;
    for (; j <= index; j++)
    {
      bytes[j] = 0;
    }
    for (j = 0; j < 8 - rest; j++) // NOTE: left padding
    {
      if (scount == 0)
      {
        module++;
        mcount++;
        scount = scale;
      }
      bytes[index] |= *module;
      scount--;
      if (j < 7 - rest)
      {
        bytes[index] <<= 1;
      }
    }
    uint16_t diff = (uint16_t)ceil((self->order_ * scale) / 8) + index;
    index++;
    for (j = index; j < nbytes; j++)
    {
      bytes[index] = 0;
      if (j <= diff)
      {
        uint8_t k = 0;
        for (; k < 8; k++)
        {
          if (scount == 0)
          {
            module++;
            mcount++;
            scount = scale;
          }
          if (mcount < self->order_)
          {
            bytes[index] |= *module;
            scount--;
          }
          if (k < 7)
          {
            bytes[index] <<= 1;
          }
        }
      }
      index++;
    }
    for (j = 0; j < scale; j++)
    {
      fwrite(bytes, 1, nbytes, file);
    }
  }
  for (i = 0; i < nbytes * 4 * scale; i++)
  {
    fputc('\0', file);
  }
  return 0;
}

void
qrmask_outsvg(qrmask_t* self, FILE* __restrict__ file)
{
  const uint32_t nmods = 8 + self->order_;
  fprintf(file,
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" __nl
    "<!-- Generated by " PROJECT_TITLE " " PROJECT_VERSION " -->" __nl
    "<svg width=\"%umm\" height=\"%umm\" viewBox=\"0 0 %u %u\" version=\"1.1\" "
      "id=\"svg1\" xmlns=\"http://www.w3.org/2000/svg\" "
      "xmlns:svg=\"http://www.w3.org/2000/svg\">" __nl
    "<defs>"
      "<rect id=\"m\" fill=\"black\" width=\"1\" height=\"1\"/>"
    "</defs>" __nl
    "<g id=\"background\">"
      "<rect fill=\"white\" width=\"%u\" height=\"%u\" x=\"0\" y=\"0\"/>"
    "</g>" __nl
    "<g id=\"modules\" transform=\"translate(4 4)\">" __nl,
    nmods, nmods, nmods, nmods, nmods, nmods);

  uint32_t row = 0;
  for (; row < self->order_; row++)
  {
    uint32_t col = 0;
    for (; col < self->order_; col++)
    {
      size_t index = row * self->order_ + col;
      if (self->v_[index] == MASK_DARK)
      {
        fprintf(file, "<use href=\"#m\" x=\"%u\" y=\"%u\"/>" __nl, col, row);
      }
    }
  }
  fprintf(file, "</g>" __nl "</svg>" __nl);
}
