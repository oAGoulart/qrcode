#include "mask.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"

#define FMTINFO_LEN 15
#define VERINFO_LEN 18
#define MIN_VERINFO_VERSION 7

#if defined(__FreeBSD__)
#define EMPTY_CHARACTER " "
#else
#define EMPTY_CHARACTER " "
#endif

extern const uint16_t* qrindex[];
extern const uint16_t* qralign[];
extern const uint16_t  qrfmtinfo[];
extern const uint32_t  qrverinfo[];

struct qrmask_s
{
  const uint16_t* i_;
  uint16_t    count_;
  uint8_t*    v_;
  eclevel_t   level_;
  uint8_t     version_;
  uint8_t     order_;
  uint8_t     pattern_;
  uint16_t    dark_;
  qrpenalty_t penalty_;
};

static __inline__ uint16_t
maskinfo_(const uint8_t pattern, const eclevel_t level)
{
  return qrfmtinfo[pattern + (uint8_t)level * 8];
}

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
      printf(EMPTY_CHARACTER);
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
      printf(EMPTY_CHARACTER);
    }
    else
    {
      printf("▀");
    }
  }
  printf("    " _nl);
}

static void __attribute__((__nonnull__))
place_finder_(const qrmask_t* self)
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
    memcpy(&self->v_[self->order_ * i], &finder[i], 7u);
    memcpy(
      &self->v_[self->order_ - 7u + self->order_ * i],
      &finder[i], 7u
    );
    memcpy(
      &self->v_[(self->order_ - 7u) * self->order_ + self->order_ * i],
      &finder[i], 7u
    );
  }
  /* NOTE: assumes separators are not required (i.e. zero-ed buffer) */
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
  const uint16_t* index = qralign[self->version_ - 1];
  while (*index != 0)
  {
    for (size_t i = 0; i < 5; i++)
    {
      memcpy(&self->v_[*index + i * self->order_],
        &align[i], 5u);
    }
    self->dark_ += 17;
    index++;
  }
}

static void __attribute__((__nonnull__))
place_format_info_(qrmask_t* self)
{
  for (uint8_t i = 0; i < FMTINFO_LEN; i++)
  {
    uint32_t idx1 = 0;
    uint32_t idx2 = 0;
    if (i < 8)
    {
      idx1 = self->order_ * 8 + i;
      idx1 += (i > 5) ? 1 : 0;
    }
    else
    {
      idx1 = (FMTINFO_LEN - i) * self->order_ + 8;
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
    const uint8_t bit = (maskinfo_(self->pattern_, self->level_) >>
      (FMTINFO_LEN - i - 1)) & 1;
    self->v_[idx1] = bit;
    self->v_[idx2] = bit;
    self->dark_ += bit * 2;
  }
}

static void __attribute__((__nonnull__))
place_version_info_(qrmask_t* self)
{
  uint32_t idx1 = 0;
  uint32_t idx2 = 0;
  for (uint8_t i = 0; i < VERINFO_LEN; i++)
  {
    if (i % 3 == 0)
    {
      idx1 = self->order_ * (self->order_ - 11) + (i / 3);
      idx2 = (self->order_ * (i / 3)) + self->order_ - 11;
    }
    else
    {
      idx1 += self->order_;
      idx2++;
    }
    const uint8_t bit = (qrverinfo[self->version_ - 6] >> i) & 1;
    self->v_[idx1] = bit;
    self->v_[idx2] = bit;
    self->dark_ += bit * 2;
  }
}

static __inline__ void __attribute__((__nonnull__))
place_infos_(qrmask_t* self)
{
  place_format_info_(self);
  if (self->version_ >= MIN_VERINFO_VERSION - 1)
  {
    place_version_info_(self);
  }
}

static void __attribute__((__nonnull__))
place_timing_(qrmask_t* self)
{
  size_t idx1 = self->order_ * 6u + 8u;
  size_t idx2 = self->order_ * 8u + 6u;
  for (size_t i = 0; i < self->order_ - 16u; i++)
  {
    const uint8_t module = (i % 2 == 0) ? MASK_DARK : MASK_LIGHT;
    self->v_[idx1] = module;
    self->v_[idx2] = module;
    if (module == MASK_DARK)
    {
      self->dark_ += 2;
    }
    idx1++;
    idx2 += self->order_;
  }
}

static __inline__ void __attribute__((__nonnull__))
place_darkmodule_(const qrmask_t* self)
{
  self->v_[(self->order_ - 8) * self->order_ + 8] = MASK_DARK;
}

static void __attribute__((__nonnull__))
percentage_penalty_(qrmask_t* self)
{
  int32_t diff = self->dark_ * 20 - self->count_ * 10;
  if (diff < 0)
  {
    diff = -diff;
  }
  int32_t k = (diff + self->count_ - 1) / self->count_ - 1;
  if (k < 0)
  {
    k = 0;
  }
  self->penalty_.balance += k * 10;
}

static __inline__ uint8_t __attribute__((__nonnull__))
finder_ratio_(const uint16_t* line)
{
  const uint16_t n = line[1];
  if (n == 0)
  {
    return 0;
  }
  /* NOTE: exact 1:1:3:1:1 proportion */
  if (line[2] == n && line[3] == n * 3 && line[4] == n && line[5] == n)
  {
    uint8_t count = 0;
    if (line[0] >= n * 4 && line[6] >= n)
    {
      count++;
    }
    if (line[6] >= n * 4 && line[0] >= n)
    {
      count++;
    }
    return count;
  }
  return 0;
}

static void __attribute__((__nonnull__))
module_penalty_(qrmask_t* self)
{
  /* NOTE: row run */
  for (uint8_t r = 0; r < self->order_; r++)
  {
    uint8_t  run_color = 0;
    uint16_t run_count = 0;
    uint16_t run_history[7] = {0};
    uint8_t  pad_left = 1;
    for (uint8_t c = 0; c < self->order_; c++)
    {
      const uint8_t color = self->v_[r * self->order_ + c];
      if (color == run_color)
      {
        run_count++;
        /* NOTE: run penalty */
        if (run_count == 5)
        {
          self->penalty_.run += 3;
        }
        else if (run_count > 5)
        {
          self->penalty_.run += 1;
        }
      }
      else
      {
        for (uint8_t i = 6; i > 0; i--)
        {
          run_history[i] = run_history[i - 1];
        }
        run_history[0] = run_count;
        if (pad_left)
        {
          run_history[0] += self->order_;
          pad_left = 0;
        }
        if (run_color == 0)
        {
          self->penalty_.finder += finder_ratio_(run_history) * 40;
        }
        run_color = color;
        run_count = 1;
      }
      if (r < self->order_ - 1 && c < self->order_ - 1)
      {
        /* NOTE: box penalty */
        const uint8_t c1 = color;
        const uint8_t c2 = self->v_[r * self->order_ + (c + 1)];
        const uint8_t c3 = self->v_[(r + 1) * self->order_ + c];
        const uint8_t c4 = self->v_[(r + 1) * self->order_ + (c + 1)];
        if (c1 == c2 && c2 == c3 && c3 == c4)
        {
          self->penalty_.box += 3;
        }
      }
    }
    for (uint8_t i = 6; i > 0; i--)
    {
      run_history[i] = run_history[i - 1];
    }
    run_history[0] = run_count;
    if (pad_left)
    {
      run_history[0] += self->order_;
    }
    if (run_color == 0)
    {
      run_history[0] += self->order_;
      self->penalty_.finder += finder_ratio_(run_history) * 40;
    }
    else
    {
      for (uint8_t i = 6; i > 0; i--)
      {
        run_history[i] = run_history[i - 1];
      }
      run_history[0] = self->order_;
      self->penalty_.finder += finder_ratio_(run_history) * 40;
    }
  }
  /* NOTE: column run (repeat as above) */
  for (uint8_t c = 0; c < self->order_; c++)
  {
    uint8_t  run_color = 0;
    uint16_t run_count = 0;
    uint16_t run_history[7] = {0};
    uint8_t  pad_top = 1;
    for (uint8_t r = 0; r < self->order_; r++)
    {
      const uint8_t color = self->v_[r * self->order_ + c];
      if (color == run_color)
      {
        run_count++;
        if (run_count == 5)
        {
          self->penalty_.run += 3;
        }
        else if (run_count > 5)
        {
          self->penalty_.run += 1;
        }
      }
      else
      {
        for (uint8_t i = 6; i > 0; i--)
        {
          run_history[i] = run_history[i - 1];
        }
        run_history[0] = run_count;
        if (pad_top)
        {
          run_history[0] += self->order_;
          pad_top = 0;
        }
        if (run_color == 0)
        {
          self->penalty_.finder += finder_ratio_(run_history) * 40;
        }
        run_color = color;
        run_count = 1;
      }
    }
    for (uint8_t i = 6; i > 0; i--)
    {
      run_history[i] = run_history[i - 1];
    }
    run_history[0] = run_count;
    if (pad_top)
    {
      run_history[0] += self->order_;
    }
    if (run_color == 0)
    {
      run_history[0] += self->order_;
      self->penalty_.finder += finder_ratio_(run_history) * 40;
    }
    else
    {
      for (uint8_t i = 6; i > 0; i--)
      {
        run_history[i] = run_history[i - 1];
      }
      run_history[0] = self->order_;
      self->penalty_.finder += finder_ratio_(run_history) * 40;
    }
  }
}

static __inline__ uint8_t __attribute__((__const__))
symbol_order_(const uint8_t version)
{
  return 4u * (version + 1u) + 17u;
}

int
create_qrmask(qrmask_t** self, const uint8_t version,
              const eclevel_t level, const uint8_t pattern)
{
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
  (*self)->version_ = version;
  (*self)->level_ = level;
  (*self)->order_ = symbol_order_(version);
  (*self)->count_ = (*self)->order_ * (*self)->order_;
  (*self)->pattern_ = pattern;
  (*self)->i_ = qrindex[version];
  (*self)->v_ = (uint8_t*)calloc((*self)->count_, 1);
  if ((*self)->v_ == NULL)
  {
    eprintf("cannot allocate %hu bytes", (*self)->count_);
    delete_qrmask(self);
    return ENOMEM;
  }
  (*self)->dark_ = 100;
  (*self)->penalty_.run = 0;
  (*self)->penalty_.box = 0;
  (*self)->penalty_.finder = 0;
  (*self)->penalty_.balance = 0;
  (*self)->penalty_.evaluated = false;
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
qrmask_set(qrmask_t* self, const uint16_t index, uint8_t module)
{
  const uint16_t idx = self->i_[index];
  if (should_xor_(self->order_, idx, self->pattern_))
  {
    module = !module;
  }
  self->v_[idx] = module;
  self->dark_ += (module == MASK_DARK);
}

qrpenalty_t
qrmask_penalty(qrmask_t* self)
{
  if (self->penalty_.evaluated == false)
  {
    place_infos_(self);
    percentage_penalty_(self);
    module_penalty_(self);
    self->penalty_.evaluated = true;
  }
  return self->penalty_;
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
    mask_single_(
      &self->v_[(self->order_ - 1) * self->order_],
      self->order_
    );
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
              const uint8_t scale, FILE* __restrict__ file)
{
  const uint32_t nbits = (8 + self->order_) * scale;
  uint32_t remain = nbits % 32u;
  const uint8_t  nlongs  = (nbits - remain) / 32u + 1u;
  const uint8_t  nbytes  = nlongs * 4;
  const uint32_t datalen = nlongs * nbits;
  const uint32_t offset  = sizeof(bitmap_t);
  const bitmap_t bm = {
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
  if (fwrite(&bm,
    sizeof(bitmap_t), 1,
    file) != 1)
  {
    eprintf("corrupted bitmap format");
    return EIO;
  }
  pdebug("writing bitmap raster data");
  for (size_t i = 0; i < (nbytes * 4 * scale); i++)
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
