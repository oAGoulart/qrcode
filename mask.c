#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include "shared.h"
#include "mask.h"

#define UNICODE_LEN 4

struct qrmask_s {
  uint8_t* v_;
  uint8_t version_;
  uint8_t order_;
  uint16_t count_;
  uint8_t masknum_;
  uint16_t dark_;
  uint16_t light_;
  uint16_t penalty_;
};

static const uint16_t
idx1_[208] = {
  440,439,419,418,398,397,377,376,356,355,335,334,
  314,313,293,292,272,271,251,250,230,229,209,208, // top
  207,206,228,227,249,248,270,269,291,290,312,311,
  333,332,354,353,375,374,396,395,417,416,438,437, // bottom
  436,435,415,414,394,393,373,372,352,351,331,330,
  310,309,289,288,268,267,247,246,226,225,205,204, // top
  203,202,224,223,245,244,266,265,287,286,308,307,
  329,328,350,349,371,370,392,391,413,412,434,433, // bottom
  432,431,411,410,390,389,369,368,348,347,327,326,
  306,305,285,284,264,263,243,242,222,221,201,200,
  180,179,159,158,117,116,96,95,75,74,54,53,33,32,12,11, // top
  10,9,31,30,52,51,73,72,94,93,115,114,157,156,178,
  177,199,198,220,219,241,240,262,261,283,282,304,
  303,325,324,346,345,367,366,388,387,409,408,430,429, // bottom
  260,259,239,238,218,217,197,196, // top
  194,193,215,214,236,235,257,256, // bottom
  255,254,234,233,213,212,192,191, // top
  190,189,211,210,232,231,253,252 // bottom
};

static const uint16_t
idx2_[359] = {
  624,623,599,598,574,573,549,548,524,523,499,498,474,473,449,448,424,423,399,
  398,374,373,349,348,324,323,299,298,274,273,249,248, // top
  247,246,272,271,297,296,322,321,347,346,372,371,397,396,422,421,447,446,472,
  471,497,496,522,521,547,546,572,571,597,596,622,621, // bottom
  620,619,595,594,570,569,545,544,395,394,370,369,345,344,320,319,295,294,270,
  269,245,244, // top
  243,242,268,267,293,292,318,317,343,342,368,367,393,392,543,542,568,567,593,
  592,618,617, // bottom
  616,615,591,590,566,565,541,540,515,490,465,440,415,391,390,366,365,341,340,
  316,315,291,290,266,265,241,240,216,215,191,190,141,140,116,115,91,90,66,65,
  41,40,16,15, // top
  14,13,39,38,64,63,89,88,114,113,139,138,189,188,214,213,239,238,264,263,289,
  288,314,313,339,338,364,363,389,388,414,413,439,438,464,463,489,488,514,513,
  539,538,564,563,589,588,614,613, // bottom
  612,611,587,586,562,561,537,536,512,511,487,486,462,461,437,436,412,411,387,
  386,362,361,337,336,312,311,287,286,262,261,237,236,212,211,187,186,137,136,
  112,111,87,86,62,61,37,36,12,11, // top
  10,9,35,34,60,59,85,84,110,109,135,134,185,184,210,209,235,234,260,259,285,
  284,310,309,335,334,360,359,385,384,410,409,435,434,460,459,485,484,510,509,
  535,534,560,559,585,584,610,609, // bottom
  408,407,383,382,358,357,333,332,308,307,283,282,258,257,233,232, // top
  230,229,255,254,280,279,305,304,330,329,355,354,380,379,405,404, // bottom
  403,402,378,377,353,352,328,327,303,302,278,277,253,252,228,227, // top
  226,225,251,250,276,275,301,300,326,325,351,350,376,375,401,400 // bottom
};

static const uint16_t*
indexes_[MAX_VERSION] = {
  (uint16_t*)&idx1_, (uint16_t*)&idx2_, NULL, NULL, NULL
};

static uint8_t
should_xor_(uint8_t order, uint16_t index, uint8_t pattern)
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
    return 0;
  }
}

static void
mask_double_(uint8_t* v, uint16_t order)
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
mask_single_(uint8_t* v, uint16_t order)
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

static void
place_finder_(qrmask_t* self)
{
  const uint8_t finder[7][7] = {
    {1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1},
  };
  size_t i = 0;
  for (; i < 7; i++)
  {
    memcpy(&self->v_[self->order_ * i], &finder[i], 7);
    memcpy(&self->v_[(self->order_ - 7) + self->order_ * i], &finder[i], 7);
    memcpy(&self->v_[(self->order_ - 7u) * self->order_ +
                     self->order_ * i], &finder[i], 7);
  }
  // NOTE: separators
  size_t idx1 = 7 * self->order_;
  size_t idx2 = 7u * self->order_ + self->order_ - 1;
  size_t idx3 = (self->order_ - 8u) * self->order_;
  for (i = 0; i < 15; i++)
  {
    if (i < 7)
    {
      self->v_[idx1++] = MASK_LIGHT;
      self->v_[idx2--] = MASK_LIGHT;
      self->v_[idx3++] = MASK_LIGHT;
    }
    else
    {
      self->v_[idx1] = MASK_LIGHT;
      self->v_[idx2] = MASK_LIGHT;
      self->v_[idx3] = MASK_LIGHT;
      idx1 -= self->order_;
      idx2 -= self->order_;
      idx3 += self->order_;
    }
  }
}

static void
place_align_(qrmask_t* self)
{
  const uint8_t align[5][5] = {
    {1, 1, 1, 1, 1},
    {1, 0, 0, 0, 1},
    {1, 0, 1, 0, 1},
    {1, 0, 0, 0, 1},
    {1, 1, 1, 1, 1}
  };
  size_t index = (self->order_ - 9u) * self->order_ + self->order_ - 9;
  size_t i = 0;
  for (; i < 5; i++)
  {
    memcpy(&self->v_[index + i * self->order_], &align[i], 5);
  }
}

static void
place_timing_(qrmask_t* self)
{
  size_t idx1 = self->order_ * 6u + 8;
  size_t idx2 = self->order_ * 8u + 6;
  size_t i = 0;
  for (; i < self->order_ - 16u; i++)
  {
    self->v_[idx1] = (i % 2 == 0) ? MASK_DARK : MASK_LIGHT;
    self->v_[idx2] = (i % 2 == 0) ? MASK_DARK : MASK_LIGHT;
    idx1++;
    idx2 += self->order_;
  }
}

static void
percentage_penalty_(qrmask_t* self)
{
  double percentage = (self->dark_ / self->count_) * 10;
  double prev = floor(percentage) * 10;
  double next = percentage - fmod(percentage * 10, 5.0) + 5;
  prev = fabs(prev - 50) / 5;
  next = fabs(next - 50) / 5;
  self->penalty_ += (uint16_t)(fmin(prev, next) * 10);
}

static void
module_penalty_(qrmask_t* self)
{
  uint16_t penalty = 0;
  uint16_t i = 0;
  for (; i < self->order_; i++)
  {
    uint16_t row = i * self->order_;
    uint16_t j = 0;
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
            penalty += 3;
          }
        }
        // NOTE: sequential line penalty (row)
        if (j < self->order_ - 4)
        {
          uint16_t count = j;
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
            penalty += (uint16_t)(3 + (count - 5));
          }
        }
      }
      // NOTE: pattern penalty (row)
      if (j < self->order_ - 10)
      {
        if (*module == MASK_LIGHT && *next == MASK_LIGHT)
        {
          if (*(module + 2) == MASK_LIGHT && *(module + 3) == MASK_LIGHT &&
              *(module + 4) == MASK_DARK && *(module + 5) == MASK_LIGHT &&
              *(module + 6) == MASK_DARK && *(module + 7) == MASK_DARK &&
              *(module + 8) == MASK_DARK && *(module + 9) == MASK_LIGHT &&
              *(module + 10) == MASK_DARK)
          {
            penalty += 40;
          }
        }
        else if (*module == MASK_DARK && *next == MASK_LIGHT)
        {
          if (*(module + 2) == MASK_DARK && *(module + 3) == MASK_DARK &&
              *(module + 4) == MASK_DARK && *(module + 5) == MASK_LIGHT &&
              *(module + 6) == MASK_DARK && *(module + 7) == MASK_LIGHT &&
              *(module + 8) == MASK_LIGHT && *(module + 9) == MASK_LIGHT &&
              *(module + 10) == MASK_LIGHT)
          {
            penalty += 40;
          }
        }
      }
    }
    // Step 2: column direction vvv
    for (j = 0; j < self->count_ - (4 * self->order_); j += self->order_)
    {
      uint8_t* module = &self->v_[i + j];
      uint8_t* next = &self->v_[i + j + self->order_];
      if (*module == *next)
      {
        // NOTE: sequential line penalty (column)
        uint16_t count = j;
        for (; next + j < self->v_ + self->count_; j += self->order_)
        {
          if (*(next + j) != *module)
          {
            break;
          }
        }
        count = (uint16_t)floor((j - count) / self->order_);
        if (count > 4)
        {
          penalty += (uint16_t)(3 + (count - 5));
        }
      }
      // NOTE: pattern penalty (column)
      if (j < self->count_ - (10 * self->order_))
      {
        if (*module == MASK_LIGHT && *next == MASK_LIGHT)
        {
          if (*(module + (2 * self->order_)) == MASK_LIGHT &&
              *(module + (3 * self->order_)) == MASK_LIGHT &&
              *(module + (4 * self->order_)) == MASK_DARK &&
              *(module + (5 * self->order_)) == MASK_LIGHT &&
              *(module + (6 * self->order_)) == MASK_DARK &&
              *(module + (7 * self->order_)) == MASK_DARK &&
              *(module + (8 * self->order_)) == MASK_DARK &&
              *(module + (9 * self->order_)) == MASK_LIGHT &&
              *(module + (10 * self->order_)) == MASK_DARK)
          {
            penalty += 40;
          }
        }
        else if (*module == MASK_DARK && *next == MASK_LIGHT)
        {
          if (*(module + (2 * self->order_)) == MASK_DARK &&
              *(module + (3 * self->order_)) == MASK_DARK &&
              *(module + (4 * self->order_)) == MASK_DARK &&
              *(module + (5 * self->order_)) == MASK_LIGHT &&
              *(module + (6 * self->order_)) == MASK_DARK &&
              *(module + (7 * self->order_)) == MASK_LIGHT &&
              *(module + (8 * self->order_)) == MASK_LIGHT &&
              *(module + (9 * self->order_)) == MASK_LIGHT &&
              *(module + (10 * self->order_)) == MASK_LIGHT)
          {
            penalty += 40;
          }
        }
      }
    }
  }
  self->penalty_ += penalty;
}

int
create_qrmask(qrmask_t** self, uint8_t version, uint8_t masknum)
{
  const uint8_t qr_order[MAX_VERSION] = {21, 25, 29, 33, 37};
  const uint16_t qr_count[MAX_VERSION] = {441, 625, 841, 1089, 1369};
  const uint16_t qr_basedark[MAX_VERSION] = {91, 112, 0, 0, 0};
  const uint16_t qr_baselight[MAX_VERSION] = {127, 139, 0, 0, 0};

  if (*self != NULL || version >= MAX_VERSION)
  {
    return EINVAL;
  }
  *self = (qrmask_t*)malloc(sizeof(qrmask_t));
  if (*self == NULL)
  {
    return ENOMEM;
  }
  (*self)->version_ = version;
  (*self)->order_ = qr_order[version];
  (*self)->count_ = qr_count[version];
  (*self)->masknum_ = masknum;
  (*self)->v_ = (uint8_t*)malloc((*self)->count_);
  if ((*self)->v_ == NULL)
  {
    free(*self);
    *self = NULL;
    return ENOMEM;
  }
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
  const uint16_t idx = indexes_[self->version_][index];
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
qrmask_penalty(qrmask_t *self)
{
  if (self->penalty_ == 0)
  {
    percentage_penalty_(self);
    module_penalty_(self);
  }
  return self->penalty_;
}

void
qrmask_apply(qrmask_t *self)
{
  const uint16_t maskinfo[NUM_MASKS] = {
    30660, 29427, 32170, 30877, 26159, 25368, 27713, 26998
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
qrmask_print(qrmask_t *self)
{
  puts("");
  uint16_t line = 0;
  for (; line < self->order_ - 1; line += 2)
  {
    mask_double_(&self->v_[line * self->order_], self->order_);
  }
  if (self->order_ % 2 != 0)
  {
    mask_single_(&self->v_[(self->order_ - 1) * self->order_], self->order_);
  }
  puts("");
}
