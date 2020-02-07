#include <litedb/storage/relation.h>
#include <litedb/storage/databasemdb.h>
#include <litedb/storage/index.h>
#include <litedb/catalog/sys_class.h>
#include <lmdb.h>
#include <litedb/utils/elog.h>
#include <assert.h>

namespace db {

class TableScanDesc {
 public:
  TableScanDesc()
      : cursor(nullptr),
        scanKey(nullptr),
        nkeys(0),
        totalFetch(0) {

  }

  ~TableScanDesc() {
    if (scanKey) {
      free(scanKey);
    }
  }

  RelationPtr rel;
  Cursor* cursor;
  ScanKey* scanKey;
  int nkeys;
  u64 totalFetch;
};

RelationPtr Relation::Create(TransactionPtr tran, u64 id) {
  std::string name = std::to_string(id);
  KVStore* table = tran->Open(name, MDB_CREATE);
  KVStoreMdb* mdb = static_cast<KVStoreMdb*>(table);
  if (!mdb->SetCompare()) {
    mdb->SetCompare(u64_cmp);
  }
  RelationPtr rel(new Relation(table));
  rel->relkind = RELKIND_RELATION;

  return rel;
}

RelationPtr Relation::OpenTable(TransactionPtr tran, u64 id) {
  std::string name = std::to_string(id);
  KVStore* table = tran->Open(name, MDB_CREATE);
  KVStoreMdb* mdb = static_cast<KVStoreMdb*>(table);
  if (!mdb->SetCompare()) {
    mdb->SetCompare(u64_cmp);
  }
  RelationPtr rel(new Relation(table));
  rel->relkind = RELKIND_RELATION;

  return rel;
}

RelationPtr Relation::OpenIndex(TransactionPtr tran, u64 id) {
  std::string name = std::to_string(id);
  KVStore* table = tran->Open(name, MDB_CREATE | MDB_DUPSORT);
  KVStoreMdb* mdb = static_cast<KVStoreMdb*>(table);
  if (!mdb->SetCompare()) {
    mdb->SetCompare(index_cmp);
  }
  RelationPtr rel(new Relation(table));
  rel->relkind = RELKIND_INDEX;
  return rel;
}

Relation::Relation(KVStore* table) :
    kvstore(table),
    relkind(0) {

}

void Relation::TableInsert(u64 id, const Tuple& tuple) {
  assert(relkind == RELKIND_RELATION);
  Slice key((char*) &id, sizeof(u64));
  Slice value;
  tuple.GetTupleData(value);
  kvstore->Put(key, value, 0);
}

i64 Relation::TableNextID() {
  assert(relkind == RELKIND_RELATION);
  Cursor* cursor = kvstore->Open();
  Slice key;
  Slice value;
  u64 id;
  if (cursor->Get(key, value, MDB_LAST)) {
    assert(key.size() == sizeof(i64));
    id = *(i64*) key.data() + 1;
  } else {
    id = 1;
  }
  kvstore->Close(cursor);
  return id;
}

i64 Relation::TableAppend(const Tuple& tuple) {
  assert(relkind == RELKIND_RELATION);
  Cursor* cursor = kvstore->Open();
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
  kvstore->Close(cursor);
  return id;
}

TableScanDescPtr TableBeginScan(RelationPtr rel, ScanKey* scanKey, int nkeys) {
  assert(rel->relkind == RELKIND_RELATION);
  TableScanDescPtr desc(new TableScanDesc());
  desc->rel = rel;
  desc->cursor = rel->kvstore->Open();
  if (scanKey && nkeys) {
    desc->nkeys = nkeys;
    size_t bytes = sizeof(ScanKey) * nkeys;
    desc->scanKey = (ScanKey*) malloc(bytes);
    memcpy(desc->scanKey, scanKey, bytes);
  }
  return desc;
}

TuplePtr TableGetNext(TableScanDescPtr scan) {
  assert(scan->rel->relkind == RELKIND_RELATION);
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

      matched = ScanKey::CheckSatisfy(scanKey, column);
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

void TableEndScan(TableScanDescPtr& scan) {
  assert(scan->rel->relkind == RELKIND_RELATION);
  scan->rel->kvstore->Close(scan->cursor);
  scan = nullptr;
}

class SysScanDesc {
  RelationPtr 	tableRel;		  /* catalog being scanned */
  RelationPtr	indexRel;	      /* NULL if doing table scan */
  TableScanDescPtr relScan;       /* only valid in storage-scan case */
  IndexScanDescPtr indexScan;     /* only valid in index-scan case */
};

SysScanDescPtr SysTableBeginScan(RelationPtr tableRel,
                                 u64 indexId,
                                 bool indexOK,
                                 ScanKey* scanKey, int nkeys) {
  SysScanDescPtr scan(new SysScanDesc());


}

TuplePtr SysTableGetNext(SysScanDescPtr scan) {
  return nullptr;
}

void SysTableEndScan(TableScanDescPtr& scan) {

}

}