#ifndef LITEDB_CATALOG_SYS_INDEX_H_
#define LITEDB_CATALOG_SYS_INDEX_H_
#include <litedb/int.h>
#include <litedb/catalog/catalog.h>
#include <litedb/storage/tuple.h>
#include <litedb/utils/vector.h>

namespace db {

struct SysIndex {
  u64 indexrelid;      /* id of the index */
  u64 indrelid;        /* id of the relation it indexes */
  i16 indnatts;        /* total number of columns in index */
  i16 indnkeyatts;     /* number of key columns in index */
  bool indisunique;    /* is this a unique index? */
  bool indisprimary;   /* is this index for primary key? */

  /* variable-length fields */
  Vector indkey;      /* column numbers of indexed cols*/


  static TuplePtr ToTuple(const SysIndex& self);
  static void InitCatalogs(std::vector<u64>& relations, std::vector<TuplePtr>& tuples);
};

#define SysIndexRelationName "sys_index"
#define SysIndexRelationId 2610

#define Anum_sys_index_indexrelid 1
#define Anum_sys_index_indrelid 2
#define Anum_sys_index_indnatts 3
#define Anum_sys_index_indnkeyatts 4
#define Anum_sys_index_indisunique 5
#define Anum_sys_index_indisprimary 6
#define Anum_sys_index_indkey 7

#define Natts_sys_index 6

}

#endif //LITEDB_CATALOG_SYS_INDEX_H_
