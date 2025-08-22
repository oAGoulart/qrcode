#include <stdlib.h>
#include <string.h>
#include "heaparray.h"
#include "shared.h"

#define PAGE_SIZE 0x200u

static __inline__ size_t __attribute__((__const__))
align_(size_t size)
{
  size_t remainder = size % PAGE_SIZE;
  return (remainder > 0) ? (size - remainder) + PAGE_SIZE : size;
}

struct harray_s
{
  size_t available_;
  uint8_t* data_ __attribute__((counted_by(available_)));
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
    memcpy(&self->data_[self->length_], obj, size);
    self->length_ += size;
    self->available_ = asize - self->length_;
  }
  else
  {
    memcpy(&self->data_[self->length_], obj, size);
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
  if (self->available_ > PAGE_SIZE)
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

__inline__ uint8_t
harray_byte(harray_t* self, const size_t index)
{
  return self->data_[index];
}
