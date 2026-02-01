#ifndef PACKEDBITS_H
#define PACKEDBITS_H 1

#include <stdint.h>

#include "bytes.h"

/* Creates "packed" bit stream from pushed bits,
   allows reading result bytes
*/
typedef struct pbits_s pbits_t;

__attribute__((__nonnull__)) int
create_pbits(pbits_t** self);
__attribute__((__nonnull__)) void
delete_pbits(pbits_t** self);

__attribute__((__nonnull__)) int
pbits_push(pbits_t* self, uint64_t value, uint8_t count);

__attribute__((__nonnull__)) int
pbits_flush(pbits_t* self);
__attribute__((__nonnull__)) bytes_t*
pbits_bytes(const pbits_t* self);

#endif
