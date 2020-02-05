#include <litedb/storage/relation.h>
#include <litedb/storage/databasemdb.h>
#include <litedb/catalog/sys_class.h>
#include <lmdb.h>
#include <litedb/utils/elog.h>
#include <assert.h>

namespace db {

class TableScanDesc {
 public:
  TableScanDesc() = default;

  Relation* rel;
  Cursor* cursor;
  ScanKey* scanKey;
  int nkeys;
  u64 totalFetch;
  TypeCmpCallback* cmp;
};

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
  Table* table = tran->Open(name, MDB_CREATE | MDB_DUPSORT);
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

void Relation::TableInsert(u64 id, const Tuple& tuple) {
  Slice key((char*) &id, sizeof(u64));
  Slice value;
  tuple.GetTupleData(value);
  table_->Put(key, value, 0);
}

i64 Relation::TableNextID() {
  Cursor* cursor = table_->Open();
  Slice key;
  Slice value;
  u64 id;
  if (cursor->Get(key, value, MDB_LAST)) {
    assert(key.size() == sizeof(i64));
    id = *(i64*) key.data() + 1;
  } else {
    id = 1;
  }
  table_->Close(cursor);
  return id;
}

i64 Relation::TableAppend(const Tuple& tuple) {
  Cursor* cursor = table_->Open();
  Slice key;
  Slice value;
  i64 id;
  if (cursor->Get(key, value, MDB_LAST)) {
    assert(key.size() == sizeof(i64));
    id = *(i64*) key.data() + 1;
  } else {
    id = 1;
  }

  key.assign((char*) &id, sizeof(id));
  tuple.GetTupleData(value);
  cursor->Put(key, value, MDB_APPEND);
  table_->Close(cursor);
  return id;
}

TableScanDescPtr Relation::TableBeginScan(ScanKey* scanKey, int nkeys) {
  TableScanDescPtr desc(new TableScanDesc());
  desc->rel = this;
  desc->cursor = table_->Open();
  desc->nkeys = nkeys;
  desc->scanKey = scanKey;
  desc->totalFetch = 0;
  if (scanKey) {
    TypeCmpCallback* cmp = GetCmpFunction(scanKey->type);
    if (!cmp) {
      elog(ERROR, "not supported type %d", scanKey->type);
    }
    desc->cmp = cmp;
  } else {
    desc->cmp = nullptr;
  }

  return desc;
}

TuplePtr Relation::TableGetNext(TableScanDescPtr& scan) {
  Slice key;
  Slice value;
  while (scan->cursor->Get(key, value, MDB_NEXT)) {
    u64 rowID = *(u64*) key.data();
    bool matched = true;

    for (int i = 0; i < scan->nkeys; ++i) {
      ScanKey* scanKey = scan->scanKey + i;

      Slice column;
      if (scanKey->attno == 0) {
        if (scanKey->type != INT8OID) {
          elog(ERROR, "invalid key type %d", scanKey->type);
        }
        column = key;
      } else {
        Tuple tuple((char*) value.data(), value.size());
        TupleMeta entry;
        tuple.Get(scanKey->attno - 1, entry);
        if (entry.type != scanKey->type) {
          elog(ERROR, "type not matched %d, %d", entry.type, scanKey->type);
        }
        column.assign(entry.data, entry.size);
      }

      matched = ScanKey::PerformCompare(scanKey, scan->cmp, column);
      if (!matched) {
        //just one column not matched
        break;
      }
    }

    if (!matched) {
      //not matched, skip this column
      continue;
    }

    //all column matched, return this tuple
    scan->totalFetch++;
    TuplePtr tuple(new Tuple((char*) value.data(), value.size()));
    tuple->SetRowID(rowID);
    return tuple;
  }
  return nullptr;
}

void Relation::TableEndScan(TableScanDescPtr& scan) {
  assert(scan->rel == this);
  scan->rel->GetTable()->Close(scan->cursor);
  scan = nullptr;
}

}