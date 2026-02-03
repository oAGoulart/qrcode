#ifndef QR_BITS_H
#define QR_BITS_H 1

#include <stdint.h>

#include "bytes.h"

/* Creates "packed" bit stream from pushed bits,
   allows reading resulting bytes
*/
typedef struct bits_s bits_t;

__attribute__((__nonnull__)) int
create_bits(bits_t** self);
__attribute__((__nonnull__)) void
delete_bits(bits_t** self);

__attribute__((__nonnull__)) int
bits_push(bits_t* self, uint64_t value, uint8_t count);

__attribute__((__nonnull__)) int
bits_flush(bits_t* self);
__attribute__((__nonnull__)) bytes_t*
bits_bytes(const bits_t* self);

#endif
