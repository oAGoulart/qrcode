#include "mask.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"

#define MASKINFO_LEN 15

extern const uint16_t qrindex[];

struct qrmask_s
{
  const uint16_t* i_;
  uint16_t  count_;
  uint8_t*  v_;
  uint8_t   version_;
  eclevel_t level_; /* TODO: move to qrdata_t */
  uint8_t   order_;
  uint8_t   pattern_;
  uint16_t  dark_;
  uint16_t  penalty_;
};

static __inline__ bool __attribute__((__const__))
should_xor_(const uint8_t order, const uint16_t index, const uint8_t pattern)
{
  const uint16_t col = index % order;
  const uint16_t row = (index - col) / order;
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
    return ((row - (row % 2)) / 2 + (col - (col % 3)) / 3) % 2 == 0;
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
  for (uint16_t i = 0; i < n; i++)
  {
    const int diff = v[i * order] - arr[i];
    if (diff != 0)
    {
      return diff;
    }
  }
  return 0;
}

static void __attribute__((__nonnull__))
mask_double_(const uint8_t* __restrict__ v, const uint8_t order)
{
  uint8_t top = 0;
  printf("    ");
  for (uint8_t bottom = order; top < order; top++, bottom++)
  {
    const uint8_t ch_case = (uint8_t)(v[top] << 1) + v[bottom];
    switch (ch_case)
    {
    case 0:
      printf(" ");
      break;
    case 1:
      printf("▄");
      break;
    case 2:
      printf("▀");
      break;
    default:
      printf("█");
      break;
    } /* switch */
  }
  printf("    " _nl);
}

static void __attribute__((__nonnull__))
mask_single_(const uint8_t* __restrict__ v, const uint8_t order)
{
  printf("    ");
  for (uint8_t top = 0; top < order; top++)
  {
    if (v[top] == 0)
    {
      printf(" ");
    }
    else
    {
      printf("▀");
    }
  }
  printf("    " _nl);
}

static void __attribute__((__nonnull__))
place_finder_(qrmask_t* self)
{
  static const uint8_t finder[7][7] = {
    { 1, 1, 1, 1, 1, 1, 1 },
    { 1, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 1, 1, 1, 0, 1 },
    { 1, 0, 1, 1, 1, 0, 1 },
    { 1, 0, 1, 1, 1, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 1 },
    { 1, 1, 1, 1, 1, 1, 1 },
  };
  for (size_t i = 0; i < 7; i++)
  {
    __builtin_memcpy(&self->v_[self->order_ * i], &finder[i], 7u);
    __builtin_memcpy(
      &self->v_[(self->order_ - 7u) + self->order_ * i],
      &finder[i], 7u
    );
    __builtin_memcpy(
      &self->v_[(self->order_ - 7u) * self->order_ + self->order_ * i],
      &finder[i], 7u
    );
  }
  /* WARNING: assumes separators are not required (zeroed buffer) */
}

static void __attribute__((__nonnull__))
place_align_(qrmask_t* self)
{
  static const uint8_t align[5][5] = {
    { 1, 1, 1, 1, 1 },
    { 1, 0, 0, 0, 1 },
    { 1, 0, 1, 0, 1 },
    { 1, 0, 0, 0, 1 },
    { 1, 1, 1, 1, 1 }
  };
  const size_t index = (self->order_ - 9u) *
                       self->order_ + self->order_ - 9u;
  for (size_t i = 0; i < 5; i++)
  {
    __builtin_memcpy(&self->v_[index + i * self->order_], &align[i], 5u);
  }
}

static void __attribute__((__nonnull__))
place_timing_(qrmask_t* self)
{
  size_t idx1 = self->order_ * 6u + 8u;
  size_t idx2 = self->order_ * 8u + 6u;
  for (size_t i = 0; i < self->order_ - 16u; i++)
  {
    self->v_[idx1] = (i % 2 == 0) ? MASK_DARK : MASK_LIGHT;
    self->v_[idx2] = (i % 2 == 0) ? MASK_DARK : MASK_LIGHT;
    idx1++;
    idx2 += self->order_;
  }
}

static __inline__ void __attribute__((__nonnull__))
place_darkmodule_(qrmask_t* self)
{
  self->v_[(self->order_ - 8) * self->order_ + 8] = MASK_DARK;
}

static void __attribute__((__nonnull__))
percentage_penalty_(qrmask_t* self)
{
  const uint32_t remain = self->dark_ % self->count_;
  const uint16_t percentage = ((self->dark_ - remain) / self->count_) * 10;
  int32_t prev = percentage * 10;
  int32_t next = percentage - (prev % 5) + 5;
  prev = (prev - 50) / 5;
  next = (next - 50) / 5;
  prev = (prev < 0) ? prev * -1 : prev;
  next = (next < 0) ? next * -1 : next;
  self->penalty_ += (prev < next) ? prev * 10 : next * 10;
}

static void __attribute__((__nonnull__))
module_penalty_(qrmask_t* self)
{
  static const uint8_t patright[9] = {
    0, 0, 0, 0, 1, 0, 1, 1, 1
  };
  static const uint8_t patleft[9] = {
    1, 1, 1, 0, 1, 0, 0, 0, 0
  };
  for (uint8_t i = 0; i < self->order_; i++)
  {
    const uint16_t row = i * self->order_;
    /* NOTE: row direction >>> */
    for (uint8_t j = 0; j < self->order_ - 1; j++)
    {
      uint8_t* module = &self->v_[row + j];
      const uint8_t* next = &self->v_[row + j + 1];
      if (*module == *next)
      {
        /* NOTE: square penalty */
        if (i < self->order_ - 1) {
          if (*(module + self->order_) == *module &&
              *(next + self->order_) == *module)
          {
            self->penalty_ += 3;
          }
        }
        /* NOTE: sequential line penalty (row) */
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
      /* NOTE: pattern penalty (row) */
      if (j < self->order_ - 10 && *next == MASK_LIGHT)
      {
        const uint8_t* pattern = (*module == MASK_DARK) ? patleft : patright;
        if (!__builtin_memcmp(module + 2, pattern, 9))
        {
          self->penalty_ += 40;
        }
      }
    }
    /* NOTE: column direction vvv */
    for (
      uint32_t k = 0;
      k < self->count_ - (4u * self->order_);
      k += self->order_
    )
    {
      const uint8_t* module = &self->v_[i + k];
      const uint8_t* next = &self->v_[i + k + self->order_];
      if (*module == *next)
      {
        /* NOTE: sequential line penalty (column) */
        uint16_t count = k;
        for (; next + k < self->v_ + self->count_; k += self->order_)
        {
          if (*(next + k) != *module)
          {
            break;
          }
        }
        const uint16_t remain = (k - count) % self->order_;
        count = (k - count - remain) / self->order_;
        if (count > 4)
        {
          self->penalty_ += (count - 5) + 3;
        }
      }
      /* NOTE: pattern penalty (column) */
      if (k < self->count_ - (10 * self->order_) && *next == MASK_LIGHT)
      {
        const uint8_t* pattern = (*module == MASK_DARK) ? patleft : patright;
        if (!colcmp_(module + (2 * self->order_),
          self->order_, 9, pattern))
        {
          self->penalty_ += 40;
        }
      }
    }
  }
}

static __inline__ uint8_t __attribute__((__const__))
symbol_order_(const uint8_t version)
{
  return 4u * (version + 1u) + 17u;
}

int
create_qrmask(qrmask_t** self, const uint8_t version, const uint8_t pattern)
{
  /* TODO: move to lookup, once higher versions available */
  static const uint16_t qr_basedark[MAX_VERSION] = {
    91u, 112u, 114u, 118u, 122u
  };
  static const uint16_t qr_offset[MAX_VERSION] = {
    0, 208u, 567u, 1134u, 1941u
  };
  if (*self != NULL)
  {
    eprintf("pointer to garbage in *self");
    return EINVAL;
  }
  if (version >= MAX_VERSION)
  {
    eprintf("invalid qrcode version: %hhu", version);
    return EINVAL;
  }
  *self = (qrmask_t*)malloc(sizeof(qrmask_t));
  if (*self == NULL)
  {
    eprintf("cannot allocate %zu bytes", sizeof(qrmask_t));
    return ENOMEM;
  }
  (*self)->version_ = version; /* OPTIMIZE: verify this is necessary */
  (*self)->order_ = symbol_order_(version);
  (*self)->count_ = (*self)->order_ * (*self)->order_;
  (*self)->pattern_ = pattern;
  (*self)->i_ = qrindex + qr_offset[version];
  (*self)->v_ = (uint8_t*)malloc((*self)->count_);
  if ((*self)->v_ == NULL)
  {
    eprintf("cannot allocate %hu bytes", (*self)->count_);
    free(*self);
    *self = NULL;
    return ENOMEM;
  }
  __builtin_memset((*self)->v_, 0, (*self)->count_);
  (*self)->dark_ = qr_basedark[version];
  (*self)->penalty_ = 0;
  place_finder_(*self);
  place_timing_(*self);
  if (version > 0)
  {
    place_align_(*self);
  }
  place_darkmodule_(*self);
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
  if (should_xor_(self->order_, idx, self->pattern_))
  {
    module = !module;
  }
  self->v_[idx] = module;
  if (module == MASK_DARK)
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
  static const uint16_t maskinfo[NUM_MASKS] = {
    30660u, 29427u, 32170u, 30877u,
    26159u, 25368u, 27713u, 26998u
  };
  for (uint8_t i = 0; i < MASKINFO_LEN; i++)
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
    self->v_[idx1] = (maskinfo[self->pattern_] >> (MASKINFO_LEN - i - 1)) & 1;
    self->v_[idx2] = (maskinfo[self->pattern_] >> (MASKINFO_LEN - i - 1)) & 1;
  }
}

void
qrmask_pbox(const qrmask_t* self)
{
  puts(_nl);
  for (uint16_t line = 0; line < self->order_ - 1; line += 2)
  {
    mask_double_(&self->v_[line * self->order_], self->order_);
  }
  if (self->order_ % 2 != 0)
  {
    mask_single_(&self->v_[(self->order_ - 1) * self->order_],
      self->order_);
  }
  puts(_nl);
}

void
qrmask_praw(const qrmask_t* self)
{
  for (size_t row = 0; row < self->order_; row++)
  {
    printf("%hhu", self->v_[row * self->order_]);
    size_t col = 1;
    for (; col < self->order_; col++)
    {
      printf(", %hhu", self->v_[row * self->order_ + col]);
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
qrmask_outbmp(const qrmask_t* self,
              const uint8_t scale,
              FILE* __restrict__ file)
{
  const uint32_t nbits = (8 + self->order_) * scale;
  uint32_t remain = nbits % 32u;
  const uint8_t nlongs = (nbits - remain) / 32u + 1u;
  const uint8_t nbytes = nlongs * 4;
  const uint32_t datalen = nlongs * nbits;
  const uint32_t offset = (uint32_t)sizeof(bitmap_t);
  bitmap_t bm = {
    { 'B', 'M' }, offset + datalen,
    0, offset, 40, nbits,
    nbits, 1, 1, 0,
    datalen, 4800, 4800, 2,
    0, {
      { -1, -1, -1, 0 },
      { 0, 0, 0, 0 }
    }
  };
  pdebug("writing bitmap header");
  if (!fwrite(&bm, sizeof(bitmap_t), 1, file))
  {
    eprintf("corrupted bitmap format");
    return EIO;
  }
  pdebug("writing bitmap raster data");
  for (size_t i = 0; i < nbytes * 4 * scale; i++)
  {
    fputc('\0', file);
  }
  for (int i = self->order_ - 1; i >= 0; i--)
  {
    const uint8_t* module = self->v_ + i * self->order_;
    uint8_t bytes[nbytes];
    uint16_t mcount = 0;
    int16_t scount = scale;
    const uint8_t rest = (4 * scale) % 8;
    uint16_t index = (uint16_t)(4 * scale - rest) / 8;
    for (uint16_t j = 0; j <= index; j++)
    {
      bytes[j] = 0;
    }
    /* NOTE: left padding */
    for (int j = 0; j < 8 - rest; j++)
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
    remain = (self->order_ * scale) % 8u;
    const uint16_t diff = (self->order_ * scale - remain) / 8u + index;
    index++;
    for (uint16_t j = index; j < nbytes; j++)
    {
      bytes[index] = 0;
      if (j <= diff)
      {
        for (uint8_t k = 0; k < 8; k++)
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
    for (uint8_t j = 0; j < scale; j++)
    {
      fwrite(bytes, 1, nbytes, file);
    }
  }
  for (uint32_t i = 0; i < nbytes * 4u * scale; i++)
  {
    fputc('\0', file);
  }
  return 0;
}

void
qrmask_outsvg(const qrmask_t* self, FILE* __restrict__ file)
{
  const uint32_t nmods = 8 + self->order_;
  fprintf(file,
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" _nl
    "<!-- Generated by " PROJECT_TITLE " " PROJECT_VERSION " -->" _nl
    "<svg width=\"%umm\" height=\"%umm\" viewBox=\"0 0 %u %u\" version=\"1.1\" "
      "id=\"svg1\" xmlns=\"http://www.w3.org/2000/svg\">" _nl
    "<defs>"
      "<rect id=\"m\" fill=\"black\" width=\"1\" height=\"1\"/>"
    "</defs>" _nl
    "<g id=\"background\">"
      "<rect fill=\"white\" width=\"%u\" height=\"%u\" x=\"0\" y=\"0\"/>"
    "</g>" _nl
    "<g id=\"modules\" transform=\"translate(4 4)\">" _nl,
    nmods, nmods, nmods, nmods, nmods, nmods
  );

  for (uint32_t row = 0; row < self->order_; row++)
  {
    for (uint32_t col = 0; col < self->order_; col++)
    {
      const size_t index = row * self->order_ + col;
      if (self->v_[index] == MASK_DARK)
      {
        fprintf(file,
          "<use href=\"#m\" x=\"%u\" y=\"%u\"/>" _nl,
          col, row);
      }
    }
  }
  fprintf(file, "</g>" _nl "</svg>" _nl);
}
