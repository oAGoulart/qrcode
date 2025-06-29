#ifndef HASHTABLES_H
#define HASHTABLES_H 1

#include <stdint.h>
#include <errno.h>

// NOTE: Galois field
#define GF_MAX UINT8_MAX
__attribute__((pure)) uint8_t
gf_alog(uint8_t exponent);
__attribute__((pure)) uint8_t
gf_log(uint8_t multiplier);

#define MAX_VERSION 5
__attribute__((pure)) uint8_t
qr_strmax(uint8_t version);
__attribute__((pure)) uint8_t
qr_order(uint8_t version);
__attribute__((pure)) uint16_t
qr_count(uint8_t version);
__attribute__((pure)) uint8_t
qr_ecclen(uint8_t version);
__attribute__((pure)) uint8_t
qr_streambytes(uint8_t version);
__attribute__((pure)) uint16_t
qr_streambits(uint8_t version);

__attribute__((pure)) uint8_t
qr_eccgen(uint8_t version, uint8_t index);
__attribute__((pure)) uint16_t
qr_index(uint8_t version, uint16_t count);

#define NUM_MASKS 8
__attribute__((pure)) uint8_t
qr_xormask(uint8_t version, uint8_t masknum, uint16_t count);

#define MASKINFO_LEN 15
__attribute__((pure)) uint16_t
qr_maskinfo(uint8_t masknum);
__attribute__((pure)) uint16_t
qr_maskpos(uint8_t side, uint8_t count);

#endif
