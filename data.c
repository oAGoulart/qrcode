#include "data.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern const uint8_t logt[];
extern const uint8_t alogt[];
extern const uint8_t rsgen[];

static __inline__ uint16_t __attribute__((__const__))
generator_offset(const uint8_t length)
{
  assert(length < 32);
  //length &= 0x1F;
  static const uint16_t offset[32] = {
    0,0,0,0,0,0,0,0,0,
    0,8,0,0,0,0,19,0,
    0,0,0,35,0,0,0,0,
    0,56,0,0,0,0,0
  };
  return offset[length];
}

struct qrdata_s
{
  size_t length_;
  uint8_t eclen_;
  uint8_t* ecc_;
};

int
create_qrdata(qrdata_t** self, const uint8_t* __restrict__ codewords,
              size_t length, uint8_t eclen)
{
  if (*self != NULL)
  {
    eprintf("pointer to garbage in *self");
    return EINVAL;
  }
  *self = (qrdata_t*)malloc(sizeof(qrdata_t));
  if (*self == NULL)
  {
    eprintf("cannot allocate %zu bytes", sizeof(qrdata_t));
    return ENOMEM;
  }
  (*self)->length_ = length;
  (*self)->eclen_ = eclen;
  const size_t maxlen = length + eclen;
  (*self)->ecc_ = (uint8_t*)malloc(maxlen);
  if ((*self)->ecc_ == NULL)
  {
    eprintf("cannot allocate %zu bytes", maxlen);
    free(*self);
    *self = NULL;
    return ENOMEM;
  }
  memcpy((*self)->ecc_, codewords, length);
  memset(&(*self)->ecc_[length], 0, eclen);
  const uint8_t* gen = rsgen + generator_offset(eclen);
  for (size_t i = 0; i < length; i++)
  {
    uint8_t lead = (*self)->ecc_[i];
    for (uint8_t j = 0; j <= eclen; j++)
    {
      (*self)->ecc_[i + j] ^= alogt[(gen[j] + logt[lead]) % UINT8_MAX];
    }
  }
  memcpy((*self)->ecc_, codewords, length);
  return 0;
}

void
delete_qrdata(qrdata_t** self)
{
  if (*self != NULL)
  {
    free((*self)->ecc_);
    free(*self);
    *self = NULL;
  }
}

__inline__ const uint8_t* const
qrdata_codewords(qrdata_t* self)
{
  return self->ecc_;
}
