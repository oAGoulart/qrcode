#ifndef QUICKRESPONSE_H
#define QUICKRESPONSE_H 1

typedef struct qrcode_s qrcode_t;

__attribute__((__nonnull__)) int
create_qrcode(qrcode_t** self, char* str);
__attribute__((__nonnull__)) void
delete_qrcode(qrcode_t** self);

__attribute__((__nonnull__)) void
qrcode_print(qrcode_t* self);

#endif
