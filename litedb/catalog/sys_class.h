#ifndef LITEDB_CATALOG_SYS_CLASS_H_
#define LITEDB_CATALOG_SYS_CLASS_H_
#include <litedb/int.h>
#include <litedb/catalog/catalog.h>
#include <litedb/storage/tuple.h>
#include <litedb/utils/misc.h>

namespace db {

struct SysClass {
  i64 relid;              /* the relation id*/
  Name relname;           /* class name */
  bool relhasindex;       /* true if has (or has had) any indexes */
  char relkind;           /* see RELKIND_xxx constants below */
  i16 relnatts;           /* number of user attributes */

  static void FromTuple(const Tuple& tuple, SysClass& self);
  static TuplePtr ToTuple(const SysClass& self);
  static i64 CreateEntry(TransactionPtr txn,
                         i64 relid,
                         const char* relname,
                         bool relhasindex,
                         char relkind,
                         i16 relnatts);

  static bool GetCatalog(TransactionPtr txn, i64 relid, SysClass* self);
  static TuplePtr GetSysClass(TransactionPtr txn, const char* relname);
};

#define  RELKIND_RELATION     'r'    /* ordinary table */
#define  RELKIND_INDEX        'i'    /* secondary index */

#define SysClassRelationName "sys_class"
#define SysClassRelationId 1259

#define Anum_sys_class_relid        0
#define Anum_sys_class_relname      1
#define Anum_sys_class_relhasindex  2
#define Anum_sys_class_relkind      3
#define Anum_sys_class_relnatts     4

#define Natts_sys_class             5

// relid index
#define sys_class_relid_index     2662

// relname index
#define sys_class_relname_index   2663

}

#endif //LITEDB_CATALOG_SYS_CLASS_H_
