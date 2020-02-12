#ifndef LITEDB_CATALOG_SYS_ATTRIBUTE_H_
#define LITEDB_CATALOG_SYS_ATTRIBUTE_H_
#include <litedb/int.h>
#include <litedb/storage/tuple.h>

namespace db {

struct SysAttribute {
  i64 attid;              /*the attribute id*/
  i64 attrelid;           /*relation containing this attribute*/
  i32 atttypid;           /*the id of the instance*/
  NameData attname;       /*name of attribute*/
  i16 attnum;             /*attnum is the "attribute number" for the attribute*/

  static TuplePtr ToTuple(const SysAttribute& self);
  static i64 CreateEntry(TransactionPtr txn,
                          i64 attrelid,
                          i32 atttypid,
                          const char* attname,
                          i16 attnum);
};

#define SysAttributeRelationName "sys_attribute"
#define SysAttributeRelationId 1249

#define Anum_sys_attribute_attid      0
#define Anum_sys_attribute_attrelid   1
#define Anum_sys_attribute_atttypid   2
#define Anum_sys_attribute_attname    3
#define Anum_sys_attribute_attnum     4

#define Natts_sys_attribute           5

}

#endif //LITEDB_CATALOG_SYS_ATTRIBUTE_H_
