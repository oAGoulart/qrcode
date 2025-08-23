#ifndef PACKEDBITS_H
#define PACKEDBITS_H 1

#include <stdint.h>
#include "heaparray.h"

typedef struct pbits_s pbits_t;

__attribute__((__nonnull__)) int
create_pbits(pbits_t** self);
__attribute__((__nonnull__)) void
delete_pbits(pbits_t** self);

__attribute__((__nonnull__, warning("bit packing may not work"))) int
pbits_push(pbits_t* self, uint64_t value, uint8_t count);

__attribute__((__nonnull__)) int
pbits_flush(pbits_t* self);
__attribute__((__nonnull__)) harray_t*
pbits_bytes(pbits_t* self);

#endif
