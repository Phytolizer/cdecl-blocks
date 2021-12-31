#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
  uint8_t** data;
  uint64_t sizeof_type;
  uint64_t* length;
  uint64_t* capacity;
} VecUnpacked;

#define VEC_UNPACK(V)                      \
  ((VecUnpacked){                          \
      .data = (uint8_t**)(&(V)->data),     \
      .sizeof_type = sizeof((V)->data[0]), \
      .length = &(V)->length,              \
      .capacity = &(V)->capacity,          \
  })

#define VEC_TYPE(T)    \
  struct {             \
    T* data;           \
    uint64_t length;   \
    uint64_t capacity; \
  }

#define VEC_FREE(V) free((V)->data)

#define VEC_PUSH(V, Value)                                               \
  (VecExpand(VEC_UNPACK(V)) ? ((V)->data[(V)->length++] = (Value), true) \
                            : false)

#define VEC_POP(V) (V)->data[--(V)->length]

bool VecExpand(VecUnpacked v);
