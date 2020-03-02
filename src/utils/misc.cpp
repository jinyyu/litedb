#include <litedb/utils/misc.h>

namespace db {

void NameSetStr(Name* name, const char* data) {
  memset(name->data, 0, sizeof(name->data));
  strncpy(name->data, data, sizeof(name->data));
}

Slice NameGetSlice(Name* name) {
  return Slice(name->data, sizeof(name->data));
}
}

