#include <litedb/storage/relation.h>
#include <litedb/storage/databasemdb.h>
#include <litedb/catalog/sys_class.h>
#include <lmdb.h>
#include <assert.h>

namespace db {

RelationPtr Relation::OpenTable(TransactionPtr tran, u64 id) {
  std::string name = std::to_string(id);
  Table* table = tran->Open(name, MDB_CREATE);
  TableMdb* mdb = static_cast<TableMdb*>(table);
  if (!mdb->SetCompare()) {
    mdb->SetCompare(u64_cmp);
  }
  RelationPtr rel(new Relation(table));
  rel->relkind_ = RELKIND_RELATION;

  return rel;
}

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

Relation::Relation(Table* table) :
    table_(table),
    relkind_(0) {

}

void Relation::InsertTuple(u64 id, const Tuple& tuple) {
  Slice key((char*) &id, sizeof(u64));
  Slice value;
  tuple.GetTupleData(value);
  table_->Put(key, value, 0);
}

u64 Relation::Append(const Tuple& tuple) {
  Cursor* cursor = table_->Open();
  Slice key;
  Slice value;
  u64 id;
  if (cursor->Get(key, value, MDB_LAST)) {
    assert(key.size() == sizeof(u64));
    id = *(u64*) key.data() + 1;
  } else {
    id = 1;
  }

  key.assign((char*) &id, sizeof(id));
  tuple.GetTupleData(value);
  cursor->Put(key, value, MDB_APPEND);
  table_->Close(cursor);
  return id;
}

}