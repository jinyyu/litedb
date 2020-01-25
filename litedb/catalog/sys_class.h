#ifndef LITEDB_CATALOG_SYS_CLASS_H_
#define LITEDB_CATALOG_SYS_CLASS_H_
#include <litedb/int.h>
#include <litedb/catalog/catalog.h>

namespace db {

struct SysClass {
  u64 id;
  char relname[NAMEDATALEN];
  bool relhasindex;
  char relkind;
};

#define SysClassRelationId 1259


}

#endif //LITEDB_CATALOG_SYS_CLASS_H_
