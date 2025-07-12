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

static const uint16_t
idx3_[567] = {
  840,839,811,810,782,781,753,752,724,723,695,694,666,665,637,636,608,607,579,
  578,550,549,521,520,492,491,463,462,434,433,405,404,376,375,347,346,318,317,
  289,288, // top
  287,286,316,315,345,344,374,373,403,402,432,431,461,460,490,489,519,518,548,
  547,577,576,606,605,635,634,664,663,693,692,722,721,751,750,780,779,809,808,
  838,837, // bottom
  836,835,807,806,778,777,749,748,575,574,546,545,517,516,488,487,459,458,430,
  429,401,400,372,371,343,342,314,313,285,284, // top
  283,282,312,311,341,340,370,369,399,398,428,427,457,456,486,485,515,514,544,
  543,573,572,747,746,776,775,805,804,834,833, // bottom
  832,831,803,802,774,773,745,744,715,686,657,628,599,571,570,542,541,513,512,
  484,483,455,454,426,425,397,396,368,367,339,338,310,309,281,280,252,251,223,
  222,165,164,136,135,107,106,78,77,49,48,20,19, // top
  18,17,47,46,76,75,105,104,134,133,163,162,221,220,250,249,279,278,308,307,337,
  336,366,365,395,394,424,423,453,452,482,481,511,510,540,539,569,568,598,597,
  627,626,656,655,685,684,714,713,743,742,772,771,801,800,830,829, // bottom
  828,827,799,798,770,769,741,740,712,711,683,682,654,653,625,624,596,595,567,
  566,538,537,509,508,480,479,451,450,422,421,393,392,364,363,335,334,306,305,
  277,276,248,247,219,218,161,160,132,131,103,102,74,73,45,44,16,15, // top
  14,13,43,42,72,71,101,100,130,129,159,158,217,216,246,245,275,274,304,303,333,
  332,362,361,391,390,420,419,449,448,478,477,507,506,536,535,565,564,594,593,
  623,622,652,651,681,680,710,709,739,738,768,767,797,796,826,825, // bottom
  824,823,795,794,766,765,737,736,708,707,679,678,650,649,621,620,592,591,563,
  562,534,533,505,504,476,475,447,446,418,417,389,388,360,359,331,330,302,301,
  273,272,244,243,215,214,157,156,128,127,99,98,70,69,41,40,12,11, // top
  10,9,39,38,68,67,97,96,126,125,155,154,213,212,242,241,271,270,300,299,329,
  328,358,357,387,386,416,415,445,444,474,473,503,502,532,531,561,560,590,589,
  619,618,648,647,677,676,706,705,735,734,764,763,793,792,822,821, // bottom
  588,587,559,558,530,529,501,500,472,471,443,442,414,413,385,384,356,355,327,
  326,298,297,269,268, // top
  266,265,295,294,324,323,353,352,382,381,411,410,440,439,469,468,498,497,527,
  526,556,555,585,584, // bottom
  583,582,554,553,525,524,496,495,467,466,438,437,409,408,380,379,351,350,322,
  321,293,292,264,263, // top
  262,261,291,290,320,319,349,348,378,377,407,406,436,435,465,464,494,493,523,
  522,552,551,581,580 // bottom
};

static const uint16_t*
indexes_[MAX_VERSION] = {
  (uint16_t*)&idx1_, (uint16_t*)&idx2_, (uint16_t*)&idx3_, NULL, NULL
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
    return FALSE;
  }
}

static int
colcmp_(const uint8_t* v, uint16_t order, const uint8_t* arr, uint16_t n)
{
  int eq = 0;
  uint16_t ui16 = 0;
  for (; ui16 < n; ui16++)
  {
    eq += v[ui16 * order] - arr[ui16];
  }
  return ui16;
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
    printf("  %s  \r\n", str);
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
  // NOTE: separators not required
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
          const uint8_t pattern[9] = {0, 0, 1, 0, 1, 1, 1, 0, 1};
          if (!memcmp(module + 2, &pattern[0], 9))
          {
            penalty += 40;
          }
        }
        else if (*module == MASK_DARK && *next == MASK_LIGHT)
        {
          const uint8_t pattern[9] = {1, 1, 1, 0, 1, 0, 0, 0, 0};
          if (!memcmp(module + 2, &pattern[0], 9))
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
          const uint8_t pattern[9] = {0, 0, 1, 0, 1, 1, 1, 0, 1};
          if (!colcmp_(module + (2 * self->order_),
                       self->order_, &pattern[0], 9))
          {
            penalty += 40;
          }
        }
        else if (*module == MASK_DARK && *next == MASK_LIGHT)
        {
          const uint8_t pattern[9] = {1, 1, 1, 0, 1, 0, 0, 0, 0};
          if (!colcmp_(module + (2 * self->order_),
                       self->order_, &pattern[0], 9))
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
  const uint16_t qr_basedark[MAX_VERSION] = {91, 112, 114, 118, 122};
  const uint16_t qr_baselight[MAX_VERSION] = {127, 139, 141, 145, 149};

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

void qrmask_raw(qrmask_t *self)
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
