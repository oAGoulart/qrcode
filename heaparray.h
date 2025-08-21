#ifndef HEAPARRAY_H
#define HEAPARRAY_H 1

#include <stdint.h>
#include <stddef.h>

typedef struct harray_s harray_t;

__attribute__((__nonnull__)) int
create_harray(harray_t** self, size_t size);
__attribute__((__nonnull__)) void
delete_harray(harray_t** self);

__attribute__((__nonnull__)) int
harray_push(harray_t* self, uint8_t* __restrict__ obj, size_t size);
__attribute__((__nonnull__)) int
harray_pop(harray_t* self, size_t size);
__attribute__((__nonnull__)) size_t
harray_length(harray_t* self);

__attribute__((__nonnull__)) uint8_t
harray_byte(harray_t* self, const size_t index);
// TODO: add short, long, quad when necessary

#endif
