#ifndef QR_DATA_H
#define QR_DATA_H 1

#include "bits.h"
#include "shared.h"

/* Creates blocks of data/ecc from string,
   allows access to codewords in the correct order group/block
*/
typedef struct qrdata_s qrdata_t;

__attribute__((__nonnull__)) int
create_qrdata(qrdata_t** self, const bits_t* __restrict__ bits,
              const qrinfo_t* const __restrict__ info);
__attribute__((__nonnull__)) void
delete_qrdata(qrdata_t** self);

#endif
