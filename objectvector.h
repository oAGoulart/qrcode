#ifndef OBJECTVECTOR_H
#define OBJECTVECTOR_H 1

#include <stddef.h>

/* Create object vector,
   allows deletion of its elements when calling delete */
typedef struct ovector_s ovector_t;

__attribute__((__nonnull__)) int
create_ovector(ovector_t** self, void (*deleter)(void**));
__attribute__((__nonnull__)) void
delete_ovector(ovector_t** self);

__attribute__((__nonnull__(1))) int
ovector_push(ovector_t* self, void* obj);
__attribute__((__nonnull__)) size_t
ovector_count(ovector_t* self);

__attribute__((__nonnull__)) void**
ovector_begin(ovector_t* self);
__attribute__((__nonnull__)) void**
ovector_end(ovector_t* self);

#endif
