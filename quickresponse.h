#ifndef QUICKRESPONSE_H
#define QUICKRESPONSE_H 1

#include "shared.h"

typedef enum imgfmt_e
{
  FMT_BMP,
  FMT_SVG
} imgfmt_t;

typedef struct qrcode_s qrcode_t;

__attribute__((__nonnull__)) int
create_qrcode(qrcode_t** self, const char* __restrict__ str,
              int vnum, bool optimize, bool verbose);
__attribute__((__nonnull__)) void
delete_qrcode(qrcode_t** self);

__attribute__((__nonnull__)) int
qrcode_forcemask(qrcode_t* self, int mask);
__attribute__((__nonnull__)) void
qrcode_print(const qrcode_t* self, bool useraw);
__attribute__((__nonnull__)) int
qrcode_output(const qrcode_t* self, imgfmt_t fmt, int scale,
              const char* __restrict__ filename);

#endif
