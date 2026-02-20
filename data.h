#ifndef QR_DATA_H
#define QR_DATA_H

#include <stddef.h>
#include <stdint.h>

#include "shared.h"

typedef struct qrdata_s qrdata_t;

__attribute__((__nonnull__)) int
create_qrdata(qrdata_t** self, const uint8_t* __restrict__ codewords,
              size_t length, uint8_t eclen);
__attribute__((__nonnull__)) void
delete_qrdata(qrdata_t** self);

__attribute__((__nonnull__)) const uint8_t* const
qrdata_codewords(qrdata_t* self);

#endif
