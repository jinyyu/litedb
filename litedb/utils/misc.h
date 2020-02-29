#ifndef LITEDB_UTILS_MISC_H_
#define LITEDB_UTILS_MISC_H_
#include <litedb/int.h>
#include <litedb/utils/slice.h>

namespace db {

void NameSetStr(Name* name, const char* data);
Slice NameGetSlice(Name* name);

static inline char* NameStr(Name* name) {
  return name->data;
}

}

#endif //LITEDB_UTILS_MISC_H_
