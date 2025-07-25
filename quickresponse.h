#ifndef QUICKRESPONSE_H
#define QUICKRESPONSE_H 1

#define GF_MAX 255

typedef struct qrcode_s qrcode_t;

__attribute__((__nonnull__)) int
create_qrcode(qrcode_t** self, char* str, uint8_t should_debug);
__attribute__((__nonnull__)) void
delete_qrcode(qrcode_t** self);

__attribute__((__nonnull__)) void
qrcode_print(qrcode_t* self, uint8_t use_raw);

#endif
