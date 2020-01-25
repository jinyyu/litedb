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

  static void InsertInitData(TransactionPtr trans);
};

#define  RELKIND_RELATION     'r'    /* ordinary table */
#define  RELKIND_INDEX        'i'    /* secondary index */

#define SysClassRelationName "SysClass"
#define SysClassRelationId 1259
#define Anum_pg_class_id 1
#define Anum_pg_class_relname 2
#define Anum_pg_class_relhasindex 3
#define Anum_pg_class_relkind 4

}

#endif //LITEDB_CATALOG_SYS_CLASS_H_
