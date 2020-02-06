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

TypeCmpCallback* GetCmpFunction(u32 type);

int i8_cmp(Entry* a, Entry* b);
int i16_cmp(Entry* a, Entry* b);
int i32_cmp(Entry* a, Entry* b);
int i64_cmp(Entry* a, Entry* b);
int u64_cmp(Entry* a, Entry* b);
int name_cmp(Entry* a, Entry* b);

int index_cmp(Entry* a, Entry* b);

}

#endif //LITEDB_UTILS_COMPARE_H_
