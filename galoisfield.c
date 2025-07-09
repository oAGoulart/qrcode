#include <errno.h>
#include <stdlib.h>
#include "galoisfield.h"
#include "bytearray.h"

struct gf_s {
  uint8_t log_[256];
  uint8_t alog_[512];
};

int
create_gf(gf_t** self, uint16_t primitive)
{
  if (*self != NULL)
  {
    return EINVAL;
  }
  *self = (gf_t*)malloc(sizeof(gf_t));
  if (*self == NULL)
  {
    return ENOMEM;
  }
  uint16_t x = 1;
  uint16_t i = 0;
  for(; i < 256; i++)
  {
    (*self)->alog_[i] = x;
    (*self)->log_[x] = i;
    x <<= 1;
    if (x & 0x100)
    {
      x ^= primitive;
    }
  }
  for(i = 255; i < 512; i++)
  {
    (*self)->alog_[i] = (*self)->alog_[i - 255];
  }
  return 0;
}

void
delete_gf(gf_t** self)
{
  if (*self != NULL)
  {
    free(*self);
    *self = NULL;
  }
}

uint8_t
gf_add(uint8_t x, uint8_t y)
{
  return x ^ y;
}

uint8_t
gf_mul(gf_t *self, uint8_t x, uint8_t y)
{
  if(x == 0 || y == 0)
  {
    return 0;
  }
  return self->alog_[self->log_[x] + self->log_[y]];
}

uint8_t
gf_pow(gf_t* self, uint8_t x, uint16_t power)
{
  return self->alog_[(self->log_[x] * power) % 255];
}

int
gf_poly_mul(gf_t* self, bytearray_t* x, bytearray_t* y, bytearray_t** out)
{
  const size_t xlen = bytearray_length(x);
  const size_t ylen = bytearray_length(y);
  int err = create_bytearray(out, xlen + ylen, 0);
  if (!err)
  {
    return err;
  }
  uint8_t* data = bytearray_data(*out);
  uint8_t* xdata = bytearray_data(x);
  uint8_t* ydata = bytearray_data(y);
  size_t i = 0;
  for (; i < ylen; i++)
  {
    size_t j = 0;
    for (; j < xlen; j++)
    {
      data[j+i] = gf_add(data[j+i], gf_mul(self, xdata[j], ydata[i]));
    }
  }
  bytearray_update(*out, ylen + xlen - 1);
}
