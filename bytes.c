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
bytes_push(bytes_t* self, const void* __restrict__ obj, size_t size)
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
  __builtin_memcpy(&self->data_[self->length_], obj, size);
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
bytes_at(const bytes_t* self, const size_t index,
          size_t size, void* out)
{
  if (index * size > self->length_)
  {
    eprintf("out of bonds index %zu", index);
    return EINVAL;
  }
  __builtin_memcpy(out, self->data_ + index * size, size);
  return 0;
}

__inline__ size_t
bytes_length(const bytes_t* self)
{
  return self->length_;
}

__inline__ void
bytes_copy(const bytes_t* self, void* dst, const size_t dstlen)
{
  size_t n = (dstlen < self->length_) ? dstlen : self->length_;
  __builtin_memcpy(dst, self->data_, n);
}

int
bytes_replace(bytes_t* self, const size_t at,
               const void* __restrict obj, size_t size)
{
  if (at + size > self->length_)
  {
    eprintf("cannot replace length %zu from index %zu, array length is %zu",
            size, at, self->length_);
    return ERANGE;
  }
  __builtin_memcpy(&self->data_[at], obj, size);
  return 0;
}

__inline__ uint8_t
bytes_byte(const bytes_t* self, const size_t index)
{
  return self->data_[index];
}

__inline__ uint16_t
bytes_short(const bytes_t* self, const size_t index)
{
  return *(uint16_t*)&self->data_[index];
}

__inline__ uint32_t
bytes_long(const bytes_t* self, const size_t index)
{
  return *(uint32_t*)&self->data_[index];
}

__inline__ uint64_t
bytes_quad(const bytes_t* self, const size_t index)
{
  return *(uint64_t*)&self->data_[index];
}

size_t
bytes_first(const bytes_t* self, const size_t from,
             void* __restrict__ obj, size_t size)
{
  if (from + size > self->length_)
  {
    eprintf("cannot find length %zu from index %zu, array length is %zu",
            size, from, self->length_);
    return ERANGE;
  }
  char* ch = __builtin_char_memchr(
    (char*)self->data_, *(char*)obj, self->length_ - size
  );
  while (ch != NULL)
  {
    const ptrdiff_t diff = (ptrdiff_t)(ch - (uintptr_t)self->data_);
    if (__builtin_memcmp(ch, obj, size) == 0)
    {
      return diff;
    }
    ch = __builtin_char_memchr(
      ch + 1, *(char*)obj, self->length_ - diff - size
    );
  }
  return -1;
}
