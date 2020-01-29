#ifndef LITEDB_UTILS_COMPARE_H_
#define LITEDB_UTILS_COMPARE_H_
#include <litedb/int.h>
#include <stdio.h>

namespace db {

struct Entry {
  size_t size;    /**< size of the data item */
  void* data;    /**< address of the data item */
};

typedef int (TypeCmpCallback)(Entry* a, Entry* b);

int u64_cmp(Entry* a, Entry* b);

}

#endif //LITEDB_UTILS_COMPARE_H_
