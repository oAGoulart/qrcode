#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "mask.h"
#include "hashtables.h"

#define MASK_DARK 1
#define MASK_LIGHT 0
#define UNICODE_LEN 4

static const uint8_t qr_modules[441] = {
  1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,
  1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,1,0,1,1,1,0,1,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,1,0,1,1,1,0,1,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,1,0,1,1,1,0,1,
  1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,
  1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
  1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

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

struct qrmask_s {
  uint8_t* v_;
  uint8_t version_;
  uint8_t masknum_;
  uint16_t dark_;
  uint16_t light_;
  uint16_t penalty_;
};

static void
percentage_penalty_(qrmask_t* self)
{
  double percentage = (self->dark_ / qr_count(self->version_)) * 10;
  double prev = floor(percentage) * 10;
  double next = percentage - fmod(percentage * 10, 5.0) + 5;
  prev = fabs(prev - 50) / 5;
  next = fabs(next - 50) / 5;
  self->penalty_ += (uint16_t)(fmin(prev, next) * 10);
}

static void
module_penalty_(qrmask_t* self)
{
  const uint8_t order = qr_order(self->version_);
  const uint16_t qrlen = qr_count(self->version_);
  uint16_t penalty = 0;
  uint16_t i = 0;
  for (; i < order; i++)
  {
    uint16_t row = i * order;
    uint16_t j = 0;
    // Step 1: row direction >>>
    for (; j < order - 1; j++)
    {
      uint8_t* module = &self->v_[row + j];
      uint8_t* next = &self->v_[row + j + 1];
      if (*module == *next)
      {
        // NOTE: square penalty
        if (i < order - 1) {
          if (*(module + order) == *module &&
              *(next + order) == *module)
          {
            penalty += 3;
          }
        }
        // NOTE: sequential line penalty (row)
        if (j < order - 4)
        {
          uint16_t count = j;
          for (; next + j + 1 < self->v_ + qrlen; j++)
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
      if (j < order - 10)
      {
        if (*module == 0 && *next == 0)
        {
          if (*(module + 2) == 0 && *(module + 3) == 0 &&
              *(module + 4) == 1 && *(module + 5) == 0 &&
              *(module + 6) == 1 && *(module + 7) == 1 &&
              *(module + 8) == 1 && *(module + 9) == 0 &&
              *(module + 10) == 1)
          {
            penalty += 40;
          }
        }
        else if (*module == 1 && *next == 0)
        {
          if (*(module + 2) == 1 && *(module + 3) == 1 &&
              *(module + 4) == 1 && *(module + 5) == 0 &&
              *(module + 6) == 1 && *(module + 7) == 0 &&
              *(module + 8) == 0 && *(module + 9) == 0 &&
              *(module + 10) == 0)
          {
            penalty += 40;
          }
        }
      }
    }
    // Step 2: column direction vvv
    for (j = 0; j < qrlen - (4 * order); j += order)
    {
      uint8_t* module = &self->v_[i + j];
      uint8_t* next = &self->v_[i + j + order];
      if (*module == *next)
      {
        // NOTE: sequential line penalty (column)
        uint16_t count = j;
        for (; next + j < self->v_ + qrlen; j += order)
        {
          if (*(next + j) != *module)
          {
            break;
          }
        }
        count = (uint16_t)floor((j - count) / order);
        if (count > 4)
        {
          penalty += (uint16_t)(3 + (count - 5));
        }
      }
      // NOTE: pattern penalty (column)
      if (j < qrlen - (10 * order))
      {
        if (*module == 0 && *next == 0)
        {
          if (*(module + (2 * order)) == 0 &&
              *(module + (3 * order)) == 0 &&
              *(module + (4 * order)) == 1 &&
              *(module + (5 * order)) == 0 &&
              *(module + (6 * order)) == 1 &&
              *(module + (7 * order)) == 1 &&
              *(module + (8 * order)) == 1 &&
              *(module + (9 * order)) == 0 &&
              *(module + (10 * order)) == 1)
          {
            penalty += 40;
          }
        }
        else if (*module == 1 && *next == 0)
        {
          if (*(module + (2 * order)) == 1 &&
              *(module + (3 * order)) == 1 &&
              *(module + (4 * order)) == 1 &&
              *(module + (5 * order)) == 0 &&
              *(module + (6 * order)) == 1 &&
              *(module + (7 * order)) == 0 &&
              *(module + (8 * order)) == 0 &&
              *(module + (9 * order)) == 0 &&
              *(module + (10 * order)) == 0)
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
  // FIXME: for v2-5
  static const uint16_t basedark[MAX_VERSION] = {106,0,0,0,0};
  static const uint16_t baselight[MAX_VERSION] = {107,0,0,0,0};
  if (*self == NULL && version > 0 && version <= MAX_VERSION)
  {
    *self = (qrmask_t*)malloc(sizeof(qrmask_t));
    if (*self == NULL)
    {
      return ENOMEM;
    }
    (*self)->version_ = --version;
    (*self)->masknum_ = masknum;
    (*self)->v_ = (uint8_t*)malloc(qr_count(version));
    if ((*self)->v_ == NULL)
    {
      free(*self);
      *self = NULL;
      return ENOMEM;
    }
    memcpy((*self)->v_, &qr_modules[0], qr_count(version));
    (*self)->dark_ = basedark[version];
    (*self)->light_ = baselight[version];
    (*self)->penalty_ = 0;
  }
  return EINVAL;
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
  uint8_t masked = (qr_xormask(0, self->masknum_, index)) ? !module : module;
  self->v_[qr_index(self->version_, index)] = masked;
  if (masked == MASK_LIGHT)
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
    // FIXME: (on Windows) segmentation fault below
    //        only after third mask?
    module_penalty_(self);
  }
  return self->penalty_;
}

void
qrmask_apply(qrmask_t *self)
{
  uint8_t i = 0;
  for (; i < MASKINFO_LEN; i++)
  {
    self->v_[qr_maskpos(0, i)] =
      (qr_maskinfo(self->masknum_) >> (MASKINFO_LEN - i - 1)) & 1;
    self->v_[qr_maskpos(1, i)] =
      (qr_maskinfo(self->masknum_) >> (MASKINFO_LEN - i - 1)) & 1;
  }
}

void
qrmask_print(qrmask_t *self)
{
  puts("");
  uint8_t order = qr_order(self->version_);
  uint16_t line = 0;
  for (; line < order - 1; line += 2)
  {
    mask_double_(&self->v_[line * order], order);
  }
  if (order % 2 != 0)
  {
    mask_single_(&self->v_[(order - 1) * order], order);
  }
  puts("");
}
