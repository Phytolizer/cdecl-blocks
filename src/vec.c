#include "vec/vec.h"

#include <stdint.h>
#include <stdlib.h>

bool VecExpand(VecUnpacked v) {
  if (*v.length + 1 > *v.capacity) {
    uint64_t n = (*v.capacity == 0) ? 1 : *v.capacity * 2;
    void* ptr = realloc(*v.data, n * v.sizeof_type);
    if (ptr == NULL) {
      return false;
    }
    *v.data = ptr;
    *v.capacity = n;
  }
  return true;
}
