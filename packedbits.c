#include <errno.h>
#include <limits.h>
#include "packedbits.h"
#include "shared.h"

static __inline__ uint8_t __attribute__((const))
extract_bits_(uint64_t value, uint8_t offset, uint8_t n)
{
  return (uint8_t)((value >> offset) & ((1 << n) - 1));
}

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
    eprintf("cannot create packedbits' heaparray");
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
    if ((*self)->array_ != NULL)
    {
      delete_harray(&(*self)->array_);
    }
    free(*self);
    *self = NULL;
  }
}

int
pbits_push(pbits_t* self, uint64_t value, uint8_t count)
{
  uint8_t offset = 0;
  if (self->bit_ > 0)
  {
    uint8_t remaider = CHAR_BIT - self->bit_;
    uint8_t n = (count < remaider) ? count : remaider;
    uint8_t bits = extract_bits_(value, offset, n);
    self->buffer_ |= bits << (CHAR_BIT - self->bit_ - n);
    self->bit_ += n;
    offset += n;
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
  while (count >= CHAR_BIT)
  {
    uint8_t byte = extract_bits_(value, offset, CHAR_BIT);
    int err = harray_push(self->array_, &byte, 1);
    if (err)
    {
      eprintf("could not push whole byte into array");
      return err;
    }
    offset += CHAR_BIT;
    count -= CHAR_BIT;
  }
  if (count > 0)
  {
    uint8_t bits = extract_bits_(value, offset, count);
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

__inline__ const harray_t*
pbits_bytes(pbits_t* self)
{
  return self->array_;
}
