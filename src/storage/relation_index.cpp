#include <litedb/storage/relation.h>
#include <litedb/catalog/sys_class.h>
#include <litedb/storage/databasemdb.h>
#include <lmdb.h>

namespace db {

RelationPtr Relation::OpenIndex(TransactionPtr tran, u64 id) {
  std::string name = std::to_string(id);
  Table* table = tran->Open(name, MDB_CREATE);
  TableMdb* mdb = static_cast<TableMdb*>(table);
  if (!mdb->SetCompare()) {
    mdb->SetCompare(index_cmp);
  }
  RelationPtr rel(new Relation(table));
  rel->relkind_ = RELKIND_INDEX;
  return rel;
}


}