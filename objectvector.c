#include "objectvector.h"

#include <stddef.h>
#include <stdlib.h>
#include "shared.h"

#define VECTOR_SIZE 0x20u

struct ovector_s
{
  void** v_;
  size_t count_;
  size_t available_;
  void (*deleter_)(void**);
};

int
ovector_create(ovector_t** self, void (*deleter)(void**))
{
  if (*self != NULL)
  {
    eprintf("pointer to garbage in *self");
    return EINVAL;
  }
  *self = (ovector_t*)malloc(sizeof(ovector_t));
  if (*self == NULL)
  {
    eprintf("cannot allocate %zu bytes", sizeof(ovector_t));
    return ENOMEM;
  }
  (*self)->available_ = VECTOR_SIZE;
  (*self)->v_ = malloc((*self)->available_ * sizeof(void*));
  if ((*self)->v_ == NULL)
  {
    eprintf("cannot allocate %zu bytes", (*self)->available_);
    free(*self);
    *self = NULL;
    return ENOMEM;
  }
  (*self)->count_ = 0;
  (*self)->deleter_ = deleter;
  return 0;
}

void
ovector_delete(ovector_t** self)
{
  if (*self != NULL)
  {
    for (void** b = (*self)->v_; b != (*self)->v_ + (*self)->available_; b++)
    {
      (*self)->deleter_(b);
    }
    free(*self);
    *self = NULL;
  }
}

int
ovector_push(ovector_t* self, void* obj)
{
  if (self->available_ == 0)
  {
    const size_t asize = self->count_ + VECTOR_SIZE;
    void** tmp = realloc(self->v_, asize * sizeof(void*));
    if (tmp == NULL)
    {
      eprintf("cannot re-allocate array to %zu bytes", asize);
      return ENOMEM;
    }
    self->v_ = tmp;
    self->available_ = VECTOR_SIZE;
  }
  self->v_[self->count_] = obj;
  self->count_++;
  self->available_--;
  return 0;
}

__inline__ size_t
ovector_count(ovector_t* self)
{
  return self->count_;
}

__inline__ void**
ovector_begin(ovector_t* self)
{
  return self->v_;
}

__inline__ void**
ovector_end(ovector_t* self)
{
  return self->v_ + self->available_;
}
