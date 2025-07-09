#include <errno.h>
#include <string.h>
#include "bytearray.h"

#define MIN_MEM 24

struct bytearray_s {
  uint8_t* data_;
  size_t size_;
  size_t count_;
};

int
create_bytearray(bytearray_t** self, size_t size, uint8_t value)
{
  if (*self != NULL)
  {
    return EINVAL;
  }
  *self = (bytearray_t*)malloc(sizeof(bytearray_t));
  if (*self == NULL)
  {
    return ENOMEM;
  }
  (*self)->size_ = (size >= MIN_MEM) ? size : MIN_MEM;
  (*self)->data_ = (uint8_t*)malloc((*self)->size_);
  if ((*self)->data_ == NULL)
  {
    free(*self);
    *self = NULL;
    return ENOMEM;
  }
  memset((*self)->data_, value, (*self)->size_);
  (*self)->count_ = 0;
  return 0;
}

void
delete_bytearray(bytearray_t** self)
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
bytearray_insert(bytearray_t* self, uint8_t value)
{
  if (self->count_ >= self->size_)
  {
    uint8_t* tmp = (uint8_t*)realloc(self->data_, self->size_ + MIN_MEM);
    if (tmp == NULL)
    {
      return ENOMEM;
    }
    self->data_ = tmp;
    self->size_ += MIN_MEM;
  }
  self->data_[self->count_] = value;
  self->count_++;
  return 0;
}

int
bytearray_remove(bytearray_t* self, uint8_t* out)
{
  if (self->count_ == 0)
  {
    return ENODATA;
  }
  if (out != NULL)
  {
    *out = self->data_[self->count_ - 1];
  }
  self->count_--;
  return 0;
}

const uint8_t*
bytearray_data(bytearray_t* self)
{
  return self->data_;
}

size_t
bytearray_length(bytearray_t* self)
{
  return self->count_;
}

void
bytearray_update(bytearray_t* self, size_t count)
{
  self->count_ = count;
}
