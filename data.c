#include "data.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern const uint8_t logt[];
extern const uint8_t alogt[];
extern const uint8_t rsgen[];

static __inline__ uint8_t
log_(const uint8_t num)
{
  /* NOTE: length 256 */
  return logt[num];
}

static __inline__ uint8_t
alog_(const uint16_t num)
{
  /* NOTE: length 512 */
  return alogt[num];
}

static __inline__ const uint8_t*
reedsolomon_(const uint8_t length)
{
  assert(length < 32);
  /*length &= 0x1F;*/
  /* A = {7,10,13,17,16,22,28,15,26,18,20,24,30} */
  static const uint16_t offset[32] = {
    0,0,0,0,0,0,0,0,0,
    0,8,0,0,19,0,33,49,
    66,84,0,103,0,124,0,147,
    0,172,0,199,0,228,0
  };
  return rsgen + offset[length];
}

struct qrdata_s
{
  size_t   length_;
  uint8_t  eclen_;
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
  const uint8_t* gen = reedsolomon_(eclen);
  for (size_t i = 0; i < length; i++)
  {
    uint8_t lead = (*self)->ecc_[i];
    for (uint8_t j = 0; j <= eclen; j++)
    {
      (*self)->ecc_[i + j] ^= alog_(gen[j] + log_(lead));
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
