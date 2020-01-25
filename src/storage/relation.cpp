#include <litedb/storage/relation.h>
#include <assert.h>

namespace db {

static int IntCompareFunc(Entry* a, Entry* b) {
  assert(a->size == sizeof(u64));
  assert(b->size == sizeof(u64));
  u64 keya = *(u64*) a->data;
  u64 keyb = *(u64*) b->data;
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
  Entry key;
  key.data = (char*) &id;
  key.size = sizeof(u64);

  Entry value;
  u32 valueLen;
  tuple.GetTupleData(&value.data, &valueLen);
  value.size = valueLen;

  table_->Put(&key, &value, 0);
}

u64 Relation::Append(const Tuple& tuple) {
  Cursor* cursor = table_->Open();
  Entry key;
  Entry value;
  u64 id = 0;
  if (cursor->Get(&key, &value, MDB_LAST)) {
    assert(key.size == sizeof(u64));
    id = *(u64*) key.data;
  }
  ++id;

  key.size = sizeof(u64);
  key.data = (char*) id;

  u32 valueLen;
  tuple.GetTupleData(&value.data, &valueLen);
  value.size = valueLen;

  if (!cursor->Put(&key, &value, 0)) {
    THROW_EXCEPTION("put tuple error");
  }
  return id;
}

}