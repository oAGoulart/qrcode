#ifndef PACKEDBITS_H
#define PACKEDBITS_H 1

#include <stdint.h>
#include "heaparray.h"

typedef struct pbits_s pbits_t;

__attribute__((__nonnull__)) int
create_pbits(pbits_t** self);
__attribute__((__nonnull__)) void
delete_pbits(pbits_t** self);

__attribute__((__nonnull__)) int
pbits_push_byte(pbits_t* self, uint8_t value, uint8_t limit);
// short
// long
// quad

__attribute__((__nonnull__)) const harray_t*
pbits_bytes(pbits_t* self);

#endif
