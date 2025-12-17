#include <stdint.h>
#ifndef QUICKRESPONSE_H
#define QUICKRESPONSE_H 1

#include "mask.h"
#include "shared.h"

/* Image format */
typedef enum imgfmt_e
{
  FMT_BMP,
  FMT_SVG
} __attribute__((packed)) imgfmt_t;

/* Creates QR code from string,
   allows print/output of generated image
*/
typedef struct qrcode_s qrcode_t;

__attribute__((__nonnull__)) int
create_qrcode(qrcode_t** self, const char* __restrict__ str,
              int version, eclevel_t level, bool optimize, bool verbose);
__attribute__((__nonnull__)) void
delete_qrcode(qrcode_t** self);

__attribute__((__nonnull__)) int
qrcode_forcemask(qrcode_t* self, int mask);

__attribute__((__nonnull__)) uint8_t
qrcode_version(const qrcode_t* self);
__attribute__((__nonnull__)) void
qrcode_print(const qrcode_t* self, bool useraw);
__attribute__((__nonnull__)) int
qrcode_output(const qrcode_t* self, imgfmt_t fmt, int scale,
              const char* __restrict__ filename);

#endif
