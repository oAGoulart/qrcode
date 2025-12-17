#include "redundantdata.h"
#include "packedbits.h"

#include <stddef.h>
#include <stdlib.h>

struct qrdata_s
{
  pbits_t** data_;
  pbits_t** ecc_;
  size_t separator;
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
  (*self)->info_ = info;
  /* TODO:
    1. instanciate array of data/ecc blocks
    2. delimit group separator index
    3. split bits into data blocks
    4. generate ecc for each data block */
  return 0;
}

void
delete_qrdata(qrdata_t** self)
{
  if (*self != NULL)
  {
    free(*self);
  }
}
