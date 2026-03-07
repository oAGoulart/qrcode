#include "bytes.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"

#define MINIMUM_BYTES_COUNT 0x20u

struct bytes_s
{
  uint8_t* data_;
  size_t   available_;
  size_t   length_;
};

int
create_bytes(bytes_t** self, const size_t size)
{
  if (*self != NULL)
  {
    eprintf("pointer to garbage in *self");
    return EINVAL;
  }
  if (size == 0)
  {
    eprintf("cannot create zero-size array");
    return EINVAL;
  }
  *self = (bytes_t*)malloc(sizeof(bytes_t));
  if (*self == NULL)
  {
    eprintf("cannot allocate %zu bytes", sizeof(bytes_t));
    return ENOMEM;
  }
  (*self)->available_ = align_memory(size, MINIMUM_BYTES_COUNT);
  (*self)->data_ = (uint8_t*)malloc((*self)->available_);
  if ((*self)->data_ == NULL)
  {
    eprintf("cannot allocate %zu bytes", (*self)->available_);
    free(*self);
    *self = NULL;
    return ENOMEM;
  }
  (*self)->length_ = 0;
  return 0;
}

void
delete_bytes(bytes_t** self)
{
  if (*self != NULL)
  {
    if ((*self)->data_ != NULL)
    {
      free((*self)->data_);
    }
    free(*self);
    *self = NULL;
  }
}

int
bytes_push(bytes_t* self, const void* __restrict__ obj, const size_t size)
{
  if (self->available_ < size)
  {
    const size_t asize = align_memory(
      self->length_ + size, MINIMUM_BYTES_COUNT
    );
    uint8_t* tmp = realloc(self->data_, asize);
    if (tmp == NULL)
    {
      eprintf("cannot re-allocate array to %zu bytes", asize);
      return ENOMEM;
    }
    self->data_ = tmp;
    self->available_ = asize - self->length_;
  }
  memcpy(&self->data_[self->length_], obj, size);
  self->length_ += size;
  self->available_ -= size;
  return 0;
}

int
bytes_pop(bytes_t* self, const size_t size)
{
  if (size > self->length_)
  {
    eprintf("cannot pop %zu bytes from array of length %zu",
            size, self->length_);
    return EINVAL;
  }
  self->length_ -= size;
  self->available_ += size;
  if (self->available_ > MINIMUM_BYTES_COUNT)
  {
    const size_t asize = align_memory(
      self->length_, MINIMUM_BYTES_COUNT
    );
    uint8_t* tmp = realloc(self->data_, asize);
    if (tmp != NULL)
    {
      /* NOTE: no error otherwise, since shrinking is optional */
      self->data_ = tmp;
      self->available_ = asize - self->length_;
    }
  }
  return 0;
}

int
bytes_at(const bytes_t* self, const size_t index, const size_t size, void* out)
{
  if (index * size > self->length_)
  {
    eprintf("out of bonds index %zu", index);
    return EINVAL;
  }
  memcpy(out, self->data_ + index * size, size);
  return 0;
}

__inline__ size_t __attribute__((__const__))
bytes_length(const bytes_t* self)
{
  return self->length_;
}

__inline__ void
bytes_copy(const bytes_t* self, void* dst, const size_t dstlen)
{
  size_t n = (dstlen < self->length_) ? dstlen : self->length_;
  memcpy(dst, self->data_, n);
}

__inline__ uint8_t __attribute__((__const__))
bytes_byte(const bytes_t* self, const size_t index)
{
  return self->data_[index];
}

__inline__ uint16_t __attribute__((__const__))
bytes_short(const bytes_t* self, const size_t index)
{
  return *(uint16_t*)&self->data_[index];
}

__inline__ uint32_t __attribute__((__const__))
bytes_long(const bytes_t* self, const size_t index)
{
  return *(uint32_t*)&self->data_[index];
}

__inline__ uint64_t __attribute__((__const__))
bytes_quad(const bytes_t* self, const size_t index)
{
  return *(uint64_t*)&self->data_[index];
}

__inline__ const uint8_t* const __attribute__((__const__))
bytes_span(const bytes_t* self, const size_t index)
{
  return &self->data_[index];
}
