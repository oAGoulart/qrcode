#ifndef BYTEARRAY_H
#define BYTEARRAY_H 1

#include <stdint.h>
#include <stdlib.h>

typedef struct bytearray_s bytearray_t;

__attribute__((__nonnull__)) int
create_bytearray(bytearray_t** self, size_t size, uint8_t value);
__attribute__((__nonnull__)) void
delete_bytearray(bytearray_t** self);

__attribute__((__nonnull__)) int
bytearray_insert(bytearray_t* self, uint8_t value);
__attribute__((__nonnull__(1))) int
bytearray_remove(bytearray_t* self, uint8_t* out);
__attribute__((__nonnull__)) const uint8_t*
bytearray_data(bytearray_t* self);
__attribute__((__nonnull__)) size_t
bytearray_length(bytearray_t* self);
__attribute__((__nonnull__)) void
bytearray_update(bytearray_t* self, size_t count);

#endif
