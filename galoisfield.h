#ifndef GALOISFIELD_H
#define GALOISFIELD_H 1

#include <stdint.h>

typedef struct gf_s gf_t;

__attribute__((__nonnull__)) int
create_gf(gf_t** self, uint16_t primitive);
__attribute__((__nonnull__)) void
delete_gf(gf_t** self);

__attribute__((__const__)) uint8_t
gf_add(uint8_t x, uint8_t y);
__attribute__((__nonnull__,__pure__)) uint8_t
gf_mul(gf_t* self, uint8_t x, uint8_t y);
__attribute__((__nonnull__,__pure__)) uint8_t
gf_pow(gf_t* self, uint8_t x, uint16_t power);

__attribute__((__nonnull__,__pure__)) int
gf_poly_mul(gf_t* self, bytearray_t* x, bytearray_t* y, bytearray_t** out);

#endif
