#include "redundantdata.h"
#include "objectvector.h"
#include "packedbits.h"

#include <stddef.h>
#include <stdlib.h>

struct qrdata_s
{
  ovector_t* data_;
  ovector_t* ecc_;
  const qrinfo_t* info_;
};

int
create_qrdata(qrdata_t** self, const pbits_t* __restrict__ bits,
              const qrinfo_t* const __restrict__ info)
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
  (*self)->info_ = info; /* OPTIMIZE: verify this is necessary */
  const size_t totalblocks = info->blocks[0] + info->blocks[1];
  (*self)->data_ = NULL;
  int err = create_ovector(
    &(*self)->data_,
    (void (*)(void**))delete_pbits);
  if (err)
  {
    eprintf("cannot create data_ member of qrdata");
    delete_qrdata(self);
    return err;
  }
  (*self)->ecc_ = NULL;
  err = create_ovector(
    &(*self)->ecc_,
    (void (*)(void**))delete_pbits);
  if (err)
  {
    eprintf("cannot create ecc_ member of qrdata");
    delete_qrdata(self);
    return err;
  }
  /* TODO:
    1. instantiate each data block's pbits_t, while:
       split `bits` into data blocks
    2. instantiate each ecc block's pbits_t, while:
       generate ecc for each data block */
  return 0;
}

void
delete_qrdata(qrdata_t** self)
{
  if (*self != NULL)
  {
    delete_ovector(&(*self)->data_);
    delete_ovector(&(*self)->ecc_);
    free(*self);
  }
}
