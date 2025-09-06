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
harray_push(harray_t* self, const void* __restrict__ obj, size_t size);
__attribute__((__nonnull__)) int
harray_pop(harray_t* self, size_t size);
__attribute__((__nonnull__(1))) int
harray_at(const harray_t* self, size_t index, size_t size, void* out);
__attribute__((__nonnull__)) size_t
harray_length(const harray_t* self);

__attribute__((__nonnull__)) void
harray_copy(const harray_t* self, void* dst, size_t dstlen);
__attribute__((__nonnull__)) int
harray_replace(harray_t* self, size_t at,
               const void* __restrict__ obj, size_t size);

__attribute__((__nonnull__)) uint8_t
harray_byte(const harray_t* self, size_t index);
__attribute__((__nonnull__)) uint16_t
harray_short(const harray_t* self, size_t index);
__attribute__((__nonnull__)) uint32_t
harray_long(const harray_t* self, size_t index);
__attribute__((__nonnull__)) uint64_t
harray_quad(const harray_t* self, size_t index);

__attribute__((__nonnull__)) size_t
harray_first(const harray_t* self, size_t from,
             void* __restrict__ obj, size_t size);

#endif
