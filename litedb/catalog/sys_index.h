#ifndef LITEDB_CATALOG_SYS_INDEX_H_
#define LITEDB_CATALOG_SYS_INDEX_H_
#include <litedb/int.h>
#include <litedb/catalog/catalog.h>
#include <litedb/storage/tuple.h>

namespace db {

struct SysIndex {
  i64 indexrelid;      /* id of the index */
  i64 indrelid;        /* id of the relation it indexes */
  i16 indnatts;        /* total number of columns in index */
  bool indisunique;    /* is this a unique index? */
  bool indisprimary;   /* is this index for primary key? */
  i16 indkey[INDEX_MAX_KEYS];  /* column numbers of indexed cols*/

  static TuplePtr ToTuple(const SysIndex& self);
  static void FromTuple(const Tuple& tuple, SysIndex& self);
  static void GetIndexList(TransactionPtr txn, i64 indrelid, std::vector<SysIndex>& index);
  static bool GetIndexTuple(TransactionPtr txn, i64 indexrelid, SysIndex& index);
  static void CreateEntry(TransactionPtr txn, const SysIndex& self);
};

#define SysIndexRelationName "sys_index"
#define SysIndexRelationId 2610

#define Anum_sys_index_indexrelid     0
#define Anum_sys_index_indrelid       1
#define Anum_sys_index_indnatts       2
#define Anum_sys_index_indisunique    3
#define Anum_sys_index_indisprimary   4
#define Anum_sys_index_indkey         5

#define Natts_sys_index               6

// indexrelid index
#define sys_index_indexrelid_index 2678

//indrelid index
#define sys_index_indrelid_index 2679

}

#endif //LITEDB_CATALOG_SYS_INDEX_H_
