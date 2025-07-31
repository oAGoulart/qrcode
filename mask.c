#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include "shared.h"
#include "mask.h"

#define UNICODE_LEN 4
#define MASKINFO_LEN 15

extern const uint16_t qrindex[];

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

static __inline__ uint8_t __attribute__ ((const))
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
    return FALSE;
  }
}

static __inline__ int
colcmp_(const uint8_t* v, uint16_t order, const uint8_t* arr, uint16_t n)
{
  uint16_t ui16 = 0;
  for (; ui16 < n; ui16++)
  {
    int diff = v[ui16 * order] - arr[ui16];
    if (diff != 0)
    {
      return diff;
    }
  }
  return 0;
}

static void
mask_double_(const uint8_t* v, uint16_t order)
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
    printf("    %s    \r\n", str);
  }
}

static void
mask_single_(const uint8_t* v, uint16_t order)
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
    printf("    %s    \r\n", &str[0]);
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
  // NOTE: separators not required (array is initialized to 0)
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
  const uint8_t patleft[9] = {1, 1, 1, 0, 1, 0, 0, 0, 0};
  const uint8_t patright[9] = {1, 1, 1, 0, 1, 0, 0, 0, 0};
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
            self->penalty_ += 3;
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
            self->penalty_ += (uint16_t)(3 + (count - 5));
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
          self->penalty_ += (uint16_t)(3 + (count - 5));
        }
      }
      // NOTE: pattern penalty (column)
      if (j < self->count_ - (10 * self->order_) && *next == MASK_LIGHT)
      {
        const uint8_t* pattern = (*module == MASK_DARK) ? patleft : patright;
        if (!colcmp_(module + (2 * self->order_), self->order_, pattern, 9))
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
  const uint8_t qr_order[MAX_VERSION] = {21u, 25u, 29u, 33u, 37u};
  const uint16_t qr_count[MAX_VERSION] = {441u, 625u, 841u, 1089u, 1369u};
  const uint16_t qr_basedark[MAX_VERSION] = {91u, 112u, 114u, 118u, 122u};
  const uint16_t qr_baselight[MAX_VERSION] = {127u, 139u, 141u, 145u, 149u};

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
  const uint16_t* qr_region[MAX_VERSION] = {
    qrindex, qrindex + 208u, qrindex + 567u, qrindex + 1134u, qrindex + 1941u
  };
  const uint16_t idx = qr_region[self->version_][index];
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
    30660u, 29427u, 32170u, 30877u, 26159u, 25368u, 27713u, 26998u
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
  puts("\r\n");
  uint16_t line = 0;
  for (; line < self->order_ - 1; line += 2)
  {
    mask_double_(&self->v_[line * self->order_], self->order_);
  }
  if (self->order_ % 2 != 0)
  {
    mask_single_(&self->v_[(self->order_ - 1) * self->order_], self->order_);
  }
  puts("\r\n");
}

void
qrmask_raw(qrmask_t *self)
{
  size_t i = 0;
  for (; i < self->order_; i++)
  {
    size_t j = 0;
    for (; j < self->order_; j++)
    {
      printf(" %d", self->v_[i * self->order_ + j]);
    }
    puts("");
  }
}
