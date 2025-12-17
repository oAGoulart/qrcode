#ifndef REDUNDANTDATA_H
#define REDUNDANTDATA_H 1

#include "packedbits.h"
#include "shared.h"

/* Creates blocks of data/ecc from string,
   allows access to codewords in the correct order group/block
*/
typedef struct qrdata_s qrdata_t;

__attribute__((__nonnull__)) int
create_qrdata(qrdata_t** self, const pbits_t* __restrict__ bits,
              const qrinfo_t* __restrict__ info);
__attribute__((__nonnull__)) void
delete_qrdata(qrdata_t** self);

#endif
