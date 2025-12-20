#include "heaparray.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"

#define ARRAY_SIZE 0x20u

struct harray_s
{
  size_t available_;
  uint8_t* data_;
  size_t length_;
};

int
create_harray(harray_t** self, const size_t size)
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
  *self = (harray_t*)malloc(sizeof(harray_t));
  if (*self == NULL)
  {
    eprintf("cannot allocate %zu bytes", sizeof(harray_t));
    return ENOMEM;
  }
  (*self)->available_ = align_memory(size, ARRAY_SIZE);
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
delete_harray(harray_t** self)
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
harray_push(harray_t* self, const void* __restrict__ obj, size_t size)
{
  if (self->available_ < size)
  {
    const size_t asize = align_memory(
      self->length_ + size, ARRAY_SIZE);
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
harray_pop(harray_t* self, const size_t size)
{
  if (size > self->length_)
  {
    eprintf("cannot pop %zu bytes from array of length %zu",
            size, self->length_);
    return EINVAL;
  }
  self->length_ -= size;
  self->available_ += size;
  if (self->available_ > ARRAY_SIZE)
  {
    const size_t asize = align_memory(
      self->length_, ARRAY_SIZE);
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
harray_at(const harray_t* self, const size_t index,
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
harray_length(const harray_t* self)
{
  return self->length_;
}

__inline__ void
harray_copy(const harray_t* self, void* dst, const size_t dstlen)
{
  size_t n = (dstlen < self->length_) ? dstlen : self->length_;
  __builtin_memcpy(dst, self->data_, n);
}

int
harray_replace(harray_t* self, const size_t at,
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
harray_byte(const harray_t* self, const size_t index)
{
  return self->data_[index];
}

__inline__ uint16_t
harray_short(const harray_t* self, const size_t index)
{
  return *(uint16_t*)&self->data_[index];
}

__inline__ uint32_t
harray_long(const harray_t* self, const size_t index)
{
  return *(uint32_t*)&self->data_[index];
}

__inline__ uint64_t
harray_quad(const harray_t* self, const size_t index)
{
  return *(uint64_t*)&self->data_[index];
}

size_t
harray_first(const harray_t* self, const size_t from,
             void* __restrict__ obj, size_t size)
{
  if (from + size > self->length_)
  {
    eprintf("cannot find length %zu from index %zu, array length is %zu",
            size, from, self->length_);
    return ERANGE;
  }
  char* ch = __builtin_char_memchr((char*)self->data_,
                                   *(char*)obj, self->length_ - size);
  while (ch != NULL)
  {
    const ptrdiff_t diff = (ptrdiff_t)(ch - (uintptr_t)self->data_);
    if (__builtin_memcmp(ch, obj, size) == 0)
    {
      return diff;
    }
    ch = __builtin_char_memchr(ch + 1, *(char*)obj,
                               self->length_ - diff - size);
  }
  return -1;
}
