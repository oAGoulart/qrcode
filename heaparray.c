#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "heaparray.h"
#include "shared.h"

#define ARRAY_SIZE 0x20u

static __inline__ size_t __attribute__((__const__))
align_(size_t size)
{
  size_t remainder = size % ARRAY_SIZE;
  return (remainder > 0) ? (size - remainder) + ARRAY_SIZE : size;
}

struct harray_s
{
  size_t available_;
  uint8_t* data_;
  size_t length_;
};

int
create_harray(harray_t** self, size_t size)
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
  (*self)->available_ = align_(size);
  (*self)->length_ = 0;
  (*self)->data_ = (uint8_t*)malloc((*self)->available_);
  if ((*self)->data_ == NULL)
  {
    eprintf("cannot allocate %zu bytes", (*self)->available_);
    free(*self);
    *self = NULL;
    return ENOMEM;
  }
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
harray_push(harray_t* self, uint8_t* __restrict__ obj, size_t size)
{
  if (self->available_ < size)
  {
    const size_t asize = align_(self->length_ + size);
    uint8_t* tmp = (uint8_t*)realloc(self->data_, asize);
    if (tmp == NULL)
    {
      eprintf("cannot re-allocate array to %zu bytes", asize);
      return ENOMEM;
    }
    self->data_ = tmp;
    __builtin_memcpy(&self->data_[self->length_], obj, size);
    self->length_ += size;
    self->available_ = asize - self->length_;
  }
  else
  {
    __builtin_memcpy(&self->data_[self->length_], obj, size);
    self->length_ += size;
    self->available_ -= size;
  }
  return 0;
}

int
harray_pop(harray_t* self, size_t size)
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
    const size_t asize = align_(self->length_);
    uint8_t* tmp = (uint8_t*)realloc(self->data_, asize);
    if (tmp != NULL)
    {
      // NOTE: no error otherwise, since shrinking is optional
      self->data_ = tmp;
      self->available_ = asize - self->length_;
    }
  }
  return 0;
}

__inline__ size_t
harray_length(harray_t* self)
{
  return self->length_;
}

__inline__ void
harray_copy(harray_t* self, uint8_t* out, size_t outlen)
{
  size_t n = (outlen < self->length_) ? outlen : self->length_;
  __builtin_memcpy(out, self->data_, n);
}

int
harray_replace(harray_t *self, size_t at, uint8_t *__restrict obj, size_t size)
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
harray_byte(harray_t* self, const size_t index)
{
  return self->data_[index];
}

__inline__ uint16_t
harray_short(harray_t* self, const size_t index)
{
  return *(uint16_t*)&self->data_[index];
}

__inline__ uint32_t
harray_long(harray_t* self, const size_t index)
{
  return *(uint32_t*)&self->data_[index];
}

__inline__ uint64_t
harray_quad(harray_t* self, const size_t index)
{
  return *(uint64_t*)&self->data_[index];
}

size_t
harray_first(harray_t* self, size_t from,
             uint8_t* __restrict__ obj, size_t size)
{
  if (from + size > self->length_)
  {
    eprintf("cannot find length %zu from index %zu, array length is %zu",
            size, from, self->length_);
    return ERANGE;
  }
  char* ch = __builtin_char_memchr((char*)self->data_,
                                   *obj, self->length_ - size);
  while (ch != NULL)
  {
    ptrdiff_t diff = (uintptr_t)ch - (uintptr_t)self->data_;
    if (__builtin_memcmp(ch, obj, size) == 0)
    {
      return diff;
    }
    ch = __builtin_char_memchr(ch + 1, *obj, self->length_ - diff - size);
  }
  return -1;
}
