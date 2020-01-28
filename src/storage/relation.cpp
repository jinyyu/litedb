#include <litedb/storage/relation.h>
#include <lmdb.h>
#include <assert.h>

namespace db {

static int IntCompareFunc(MDB_val* a, MDB_val* b) {
  assert(a->mv_size == sizeof(u64));
  assert(b->mv_size == sizeof(u64));
  u64 keya = *(u64*) a->mv_data;
  u64 keyb = *(u64*) b->mv_data;
  if (keya == keyb) {
    return 0;
  }
  if (keya < keyb) {
    return -1;
  } else {
    return 1;
  }
}

RelationPtr Relation::OpenTable(TransactionPtr tran, u64 id) {
  std::string name = std::to_string(id);
  Table* table = tran->Open(name, MDB_CREATE);
  table->SetCompare(IntCompareFunc);
  return RelationPtr(new Relation(table));
}

Relation::Relation(Table* table) :
    table_(table) {

}

void Relation::InsertTuple(u64 id, const Tuple& tuple) {
  Slice key((char*) &id, sizeof(u64));
  Slice value;
  tuple.GetTupleData(value);
  table_->Put(key, value, 0);
}

u64 Relation::Append(const Tuple& tuple) {
  Cursor* cursor = table_->Open();
  assert(false);
  return 0;
}

}