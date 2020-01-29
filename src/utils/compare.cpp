#include <litedb/utils/compare.h>
#include <assert.h>

namespace db {

int u64_cmp(Entry* a, Entry* b) {
  assert(a->size == sizeof(u64) && b->size == sizeof(u64));
  u64 va = *(u64*) a->data;
  u64 vb = *(u64*) b->data;
  if (va == vb) {
    return 0;
  } else if (va > vb) {
    return 1;
  } else {
    return -1;
  }
}

}



