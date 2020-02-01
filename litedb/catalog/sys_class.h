#ifndef LITEDB_CATALOG_SYS_CLASS_H_
#define LITEDB_CATALOG_SYS_CLASS_H_
#include <litedb/int.h>
#include <litedb/catalog/catalog.h>
#include <litedb/storage/tuple.h>

namespace db {

struct SysClass {
  i64 id;
  char relname[NAMEDATALEN];  /* class name */
  bool relhasindex;           /* true if has (or has had) any indexes */
  char relkind;               /* see RELKIND_xxx constants below */
  i16 relnatts;               /* number of user attributes */

  static void FromTuple(const Tuple& tuple, SysClass& self);
  static TuplePtr ToTuple(const SysClass& self);
  static void InitCatalogs(TransactionPtr txn);
  static void CreateEntry(TransactionPtr txn,
                          i64 id,
                          const char* relname,
                          bool relhasindex,
                          char relkind,
                          i16 relnatts);
};

#define  RELKIND_RELATION     'r'    /* ordinary table */
#define  RELKIND_INDEX        'i'    /* secondary index */

#define SysClassRelationName "sys_class"
#define SysClassRelationId 1259

#define Anum_sys_class_id 1
#define Anum_sys_class_relname 2
#define Anum_sys_class_relhasindex 3
#define Anum_sys_class_relkind 4
#define Anum_sys_class_relnatts 5

#define Natts_sys_class 5

}

#endif //LITEDB_CATALOG_SYS_CLASS_H_
