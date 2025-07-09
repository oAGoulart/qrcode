#ifndef MASK_H
#define MASK_H 1

#include <stdint.h>

#define MASK_DARK 1
#define MASK_LIGHT 0

typedef struct qrmask_s qrmask_t;

__attribute__((__nonnull__)) int
create_qrmask(qrmask_t** self, uint8_t version, uint8_t masknum);
__attribute__((__nonnull__)) void
delete_qrmask(qrmask_t** self);

// NOTE: index is for data position lookup table, NOT the module matrix.
__attribute__((__nonnull__)) void
qrmask_set(qrmask_t* self, uint16_t index, uint8_t module);

__attribute__((__nonnull__)) uint16_t
qrmask_penalty(qrmask_t* self);

__attribute__((__nonnull__)) void
qrmask_apply(qrmask_t* self);
__attribute__((__nonnull__)) void
qrmask_print(qrmask_t* self);

#endif
