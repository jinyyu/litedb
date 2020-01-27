#ifndef LITEDB_CATALOG_SYS_CLASS_H_
#define LITEDB_CATALOG_SYS_CLASS_H_
#include <litedb/int.h>
#include <litedb/catalog/catalog.h>
#include <litedb/storage/tuple.h>

namespace db {

struct SysClass {
  u64 id;
  char relname[NAMEDATALEN];
  bool relhasindex;
  char relkind;

  static TuplePtr ToTuple(const SysClass& self);

  static void InitCatalogs(std::vector<u64>& relations, std::vector<TuplePtr>& tuples);
};

#define  RELKIND_RELATION     'r'    /* ordinary table */
#define  RELKIND_INDEX        'i'    /* secondary index */

#define SysClassRelationName "sys_class"
#define SysClassRelationId 1259
#define Anum_sys_class_id 0
#define Anum_sys_class_relname 1
#define Anum_sys_class_relhasindex 2
#define Anum_sys_class_relkind 3

}

#endif //LITEDB_CATALOG_SYS_CLASS_H_
