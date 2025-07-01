#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include "shared.h"
#include "mask.h"

#define MASK_DARK 1
#define MASK_LIGHT 0
#define UNICODE_LEN 4

static const uint8_t
order_[MAX_VERSION] = {21,25,29,33,37};
static const uint16_t
count_[MAX_VERSION] = {441,625,841,1089,1369};

// FIXME: for v3-5
/*static const uint16_t
numbits_[MAX_VERSION] = {208,359,0,0,0};*/
static const uint16_t
basedark_[MAX_VERSION] = {91,112,0,0,0};
static const uint16_t
baselight_[MAX_VERSION] = {127,139,0,0,0};

static const uint8_t
m1_[441] = {
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

static const uint8_t
m2_[625] = {
  1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,
  1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1,0,1,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1,0,1,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1,0,1,
  1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,
  1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,
  0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,
  1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,
  1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static const uint8_t*
modules_[MAX_VERSION] = {
  (uint8_t*)&m1_, (uint8_t*)&m2_, NULL, NULL, NULL
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

static const uint16_t
maskinfo_[NUM_MASKS] = {
  30660,29427,32170,30877,26159,25368,27713,26998
};

// NOTE: maskinfo is duplicated into two different positions
static const uint16_t
maskpos_[2][2][MASKINFO_LEN] = {
  {
    {
      168,169,170,171,172,173,175,176,155,113,92,71,50,29,8
    },
    {
      428,407,386,365,344,323,302,181,182,183,184,185,186,187,188
    }
  },
  {
    {
      200,201,202,203,204,205,207,208,183,133,108,83,58,33,8
    },
    {
      608,583,558,533,508,483,458,217,218,219,220,221,222,223,224
    }
  }
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
    return (uint16_t)(floor(row / 2) + floor(col / 3)) % 2 == 0;
  case 5:
    return ((row * col) % 2) + ((row * col) % 3) == 0;
  case 6:
    return (((row * col) % 2) + ((row * col) % 3) ) % 2 == 0;
  case 7:
    return (((row + col) % 2) + ((row * col) % 3) ) % 2 == 0;
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
  double percentage = (self->dark_ / count_[self->version_]) * 10;
  double prev = floor(percentage) * 10;
  double next = percentage - fmod(percentage * 10, 5.0) + 5;
  prev = fabs(prev - 50) / 5;
  next = fabs(next - 50) / 5;
  self->penalty_ += (uint16_t)(fmin(prev, next) * 10);
}

static void
module_penalty_(qrmask_t* self)
{
  const uint8_t order = order_[self->version_];
  const uint16_t qrlen = count_[self->version_];
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
  if (*self != NULL && version == 0 && version > MAX_VERSION)
  {
    return EINVAL;
  }
  *self = (qrmask_t*)malloc(sizeof(qrmask_t));
  if (*self == NULL)
  {
    return ENOMEM;
  }
  (*self)->version_ = version;
  (*self)->masknum_ = masknum;
  (*self)->v_ = (uint8_t*)malloc(count_[version]);
  if ((*self)->v_ == NULL)
  {
    free(*self);
    *self = NULL;
    return ENOMEM;
  }
  memcpy((*self)->v_, &modules_[version][0], count_[version]);
  (*self)->dark_ = basedark_[version];
  (*self)->light_ = baselight_[version];
  (*self)->penalty_ = 0;
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
  if (should_xor_(order_[self->version_], index, self->masknum_))
  {
    module = (module) ? 0 : 1;
  }
  self->v_[indexes_[self->version_][index]] = module;
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
  uint8_t i = 0;
  for (; i < MASKINFO_LEN; i++)
  {
    self->v_[maskpos_[self->version_][0][i]] =
      (maskinfo_[self->masknum_] >> (MASKINFO_LEN - i - 1)) & 1;
    self->v_[maskpos_[self->version_][1][i]] =
      (maskinfo_[self->masknum_] >> (MASKINFO_LEN - i - 1)) & 1;
  }
}

void
qrmask_print(qrmask_t *self)
{
  puts("");
  const uint8_t order = order_[self->version_];
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
