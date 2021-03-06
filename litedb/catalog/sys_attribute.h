#ifndef LITEDB_CATALOG_SYS_ATTRIBUTE_H_
#define LITEDB_CATALOG_SYS_ATTRIBUTE_H_
#include <litedb/int.h>
#include <litedb/storage/tuple.h>

namespace db {

struct SysAttribute {
  i64 attid;              /*the attribute id*/
  i64 attrelid;           /*relation containing this attribute*/
  i32 atttypid;           /*the id of the instance*/
  Name attname;           /*name of attribute*/
  i16 attnum;             /*attnum is the "attribute number" for the attribute*/

  static void FromTuple(const Tuple& tuple, SysAttribute& self);
  static TuplePtr ToTuple(const SysAttribute& self);
  static i64 CreateEntry(TransactionPtr txn,
                          i64 attrelid,
                          i32 atttypid,
                          const char* attname,
                          i16 attnum);

  static void GetAttributeList(TransactionPtr txn, i64 attrelid, i16 relnatts, std::vector<SysAttribute>& atrrs);
};

#define SysAttributeRelationName "sys_attribute"
#define SysAttributeRelationId 1249

#define Anum_sys_attribute_attid      0
#define Anum_sys_attribute_attrelid   1
#define Anum_sys_attribute_atttypid   2
#define Anum_sys_attribute_attname    3
#define Anum_sys_attribute_attnum     4

#define Natts_sys_attribute           5

// attid index
#define sys_attribute_attid_index 2657

//  (attrelid, attname) index
#define sys_attribute_attrelid_attname_index 2658

// (attrelid, attnum) index
#define sys_attribute_attrelid_attnum_index  2659

}

#endif //LITEDB_CATALOG_SYS_ATTRIBUTE_H_
