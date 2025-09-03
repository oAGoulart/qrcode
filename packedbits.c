#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include "packedbits.h"
#include "shared.h"

struct pbits_s
{
  harray_t* array_;
  uint8_t buffer_;
  uint8_t bit_;
};

int
create_pbits(pbits_t** self)
{
  if (*self != NULL)
  {
    eprintf("pointer to garbage in *self");
    return EINVAL;
  }
  *self = (pbits_t*)malloc(sizeof(pbits_t));
  if (*self == NULL)
  {
    eprintf("cannot allocate %zu bytes", sizeof(pbits_t));
    return ENOMEM;
  }
  (*self)->array_ = NULL;
  int err = create_harray(&(*self)->array_, 1);
  if (err)
  {
    eprintf("cannot create heaparray member of packedbits");
    free(*self);
    *self = NULL;
    return err;
  }
  (*self)->bit_ = 0;
  (*self)->buffer_ = 0;
  return 0;
}

void
delete_pbits(pbits_t** self)
{
  if (*self != NULL)
  {
    delete_harray(&(*self)->array_);
    free(*self);
    *self = NULL;
  }
}

int
pbits_push(pbits_t* self, uint64_t value, uint8_t count)
{
  if (self->bit_ > 0)
  {
    uint8_t remaider = CHAR_BIT - self->bit_;
    uint8_t n = (count < remaider) ? count : remaider;
    uint8_t bits = (uint8_t)(value >> (count - n));
    //bits &= 0xFF >> (CHAR_BIT - n);
    self->buffer_ |= bits << (CHAR_BIT - self->bit_ - n);
    self->bit_ += n;
    count -= n;
    if (self->bit_ == CHAR_BIT)
    {
      int err = harray_push(self->array_, &self->buffer_, 1);
      if (err)
      {
        eprintf("could not push byte buffer into array");
        return err;
      }
      self->buffer_ = 0;
      self->bit_ = 0;
    }
  }
  uint8_t offset = (count <= CHAR_BIT) ? 0 : count - CHAR_BIT;
  while (count >= CHAR_BIT)
  {
    uint8_t byte = (uint8_t)(value >> offset);
    int err = harray_push(self->array_, &byte, 1);
    if (err)
    {
      eprintf("could not push whole byte into array");
      return err;
    }
    count -= CHAR_BIT;
    offset = (count <= CHAR_BIT) ? 0 : count - CHAR_BIT;
  }
  if (count > 0)
  {
    uint8_t bits = (uint8_t)value & (0xFF >> (CHAR_BIT - count));
    self->buffer_ = bits << (CHAR_BIT - count);
    self->bit_ = count;
  }
  return 0;
}

int
pbits_flush(pbits_t* self)
{
  if (self->bit_ > 0)
  {
    int err = harray_push(self->array_, &self->buffer_, 1);
    if (err)
    {
      eprintf("could not push byte buffer into array");
      return err;
    }
    self->buffer_ = 0;
    self->bit_ = 0;
  }
  return 0;
}

__inline__ harray_t*
pbits_bytes(pbits_t* self)
{
  return self->array_;
}
