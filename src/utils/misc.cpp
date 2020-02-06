#include <litedb/utils/misc.h>

namespace db {

void NameDataSetStr(NameData* name, const char* data) {
  memset(name->data, 0, sizeof(name->data));
  strncpy(name->data, data, sizeof(name->data));
}

Slice NameDataGetSlice(NameData* name) {
  return Slice(name->data, sizeof(name->data));
}
}

