#ifndef QR_MASK_H
#define QR_MASK_H 1

#include <stdint.h>
#include <stdio.h>

#include "shared.h"

#define MASK_DARK  1
#define MASK_LIGHT 0

typedef struct qrmask_s qrmask_t;

typedef struct qrpenalty_s
{
  uint16_t run;
  uint16_t box;
  uint16_t finder;
  uint16_t balance;
  bool evaluated;
} qrpenalty_t;

__attribute__((__nonnull__)) int
create_qrmask(qrmask_t** self, uint8_t version,
              eclevel_t level, uint8_t pattern);
__attribute__((__nonnull__)) void
delete_qrmask(qrmask_t** self);

__attribute__((__nonnull__)) void
qrmask_set(qrmask_t* self, uint16_t index, uint8_t module);
__attribute__((__nonnull__)) qrpenalty_t
qrmask_penalty(qrmask_t* self);

__attribute__((__nonnull__)) void
qrmask_pbox(const qrmask_t* self);
__attribute__((__nonnull__)) void
qrmask_praw(const qrmask_t* self);

__attribute__((__nonnull__)) int
qrmask_outbmp(const qrmask_t* self,
              uint8_t scale, FILE* __restrict__ file);
__attribute__((__nonnull__)) void
qrmask_outsvg(const qrmask_t* self, FILE* __restrict__ file);

#endif
