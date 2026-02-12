#include "bits.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "shared.h"

struct bits_s
{
  bytes_t* data_;
  uint8_t  buffer_;
  uint8_t  bit_;
};

int
create_bits(bits_t** self)
{
  if (*self != NULL)
  {
    eprintf("pointer to garbage in *self");
    return EINVAL;
  }
  *self = (bits_t*)malloc(sizeof(bits_t));
  if (*self == NULL)
  {
    eprintf("cannot allocate %zu bytes", sizeof(bits_t));
    return ENOMEM;
  }
  (*self)->data_ = NULL;
  const int err = create_bytes(&(*self)->data_, 1);
  if (err)
  {
    eprintf("cannot create data_ member of bits_t");
    free(*self);
    *self = NULL;
    return err;
  }
  (*self)->bit_ = 0;
  (*self)->buffer_ = 0;
  return 0;
}

void
delete_bits(bits_t** self)
{
  if (*self != NULL)
  {
    delete_bytes(&(*self)->data_);
    free(*self);
    *self = NULL;
  }
}

int
bits_push(bits_t* self, const uint64_t value, uint8_t count)
{
  if (self->bit_ > 0)
  {
    const uint8_t remainder = CHAR_BIT - self->bit_;
    const uint8_t n = (count < remainder) ? count : remainder;
    const uint8_t bits = (uint8_t)(value >> (count - n));
    self->buffer_ |= bits << (CHAR_BIT - self->bit_ - n);
    self->bit_ += n;
    count -= n;
    if (self->bit_ == CHAR_BIT)
    {
      const int err = bytes_push(self->data_, &self->buffer_, 1);
      if (err)
      {
        eprintf("could not push byte into array");
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
    const int err = bytes_push(self->data_, &byte, 1);
    if (err)
    {
      eprintf("could not push byte into array");
      return err;
    }
    count -= CHAR_BIT;
    offset = (count <= CHAR_BIT) ? 0 : count - CHAR_BIT;
  }
  if (count > 0)
  {
    const uint8_t bits = (uint8_t)value & (0xFF >> (CHAR_BIT - count));
    self->buffer_ = bits << (CHAR_BIT - count);
    self->bit_ = count;
  }
  return 0;
}

int
bits_flush(bits_t* self)
{
  if (self->bit_ > 0)
  {
    const int err = bytes_push(self->data_, &self->buffer_, 1);
    if (err)
    {
      eprintf("could not push byte into array");
      return err;
    }
    self->buffer_ = 0;
    self->bit_ = 0;
  }
  return 0;
}

__inline__ bytes_t*
bits_bytes(const bits_t* self)
{
  return self->data_;
}
