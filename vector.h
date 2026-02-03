#ifndef QR_VECTOR_H
#define QR_VECTOR_H 1

#include <stddef.h>

/* Create object vector,
   allows deletion of its elements when calling delete */
typedef struct vector_s vector_t;

__attribute__((__nonnull__)) int
create_vector(vector_t** self, void (*deleter)(void**));
__attribute__((__nonnull__)) void
delete_vector(vector_t** self);

__attribute__((__nonnull__(1))) int
vector_push(vector_t* self, void* obj);
__attribute__((__nonnull__)) size_t
vector_count(vector_t* self);

__attribute__((__nonnull__)) void**
vector_begin(vector_t* self);
__attribute__((__nonnull__)) void**
vector_end(vector_t* self);

#endif
