#ifndef QR_BYTES_H
#define QR_BYTES_H 1

#include <stdint.h>
#include <stddef.h>

typedef struct bytes_s bytes_t;

__attribute__((__nonnull__)) int
create_bytes(bytes_t** self, size_t size);
__attribute__((__nonnull__)) void
delete_bytes(bytes_t** self);

__attribute__((__nonnull__)) int
bytes_push(bytes_t* self, const void* __restrict__ obj, size_t size);
__attribute__((__nonnull__)) int
bytes_pop(bytes_t* self, size_t size);
__attribute__((__nonnull__)) int
bytes_at(const bytes_t* self, size_t index, size_t size, void* out);
__attribute__((__nonnull__)) size_t
bytes_length(const bytes_t* self);

__attribute__((__nonnull__)) void
bytes_copy(const bytes_t* self, void* dst, size_t dstlen);

__attribute__((__nonnull__)) uint8_t
bytes_byte(const bytes_t* self, const size_t index);
__attribute__((__nonnull__)) uint16_t
bytes_short(const bytes_t* self, const size_t index);
__attribute__((__nonnull__)) uint32_t
bytes_long(const bytes_t* self, const size_t index);
__attribute__((__nonnull__)) uint64_t
bytes_quad(const bytes_t* self, const size_t index);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-qualifiers"
__attribute__((__nonnull__)) const uint8_t* const
bytes_span(const bytes_t* self, const size_t index);
#pragma clang diagnostic pop

#endif
