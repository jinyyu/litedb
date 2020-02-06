#ifndef LITEDB_UTILS_MISC_H_
#define LITEDB_UTILS_MISC_H_
#include <litedb/int.h>
#include <litedb/utils/slice.h>

namespace db {

void NameDataSetStr(NameData* name, const char* data);
Slice NameDataGetSlice(NameData* name);

}

#endif //LITEDB_UTILS_MISC_H_
