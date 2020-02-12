#include <litedb/storage/relation.h>
#include <litedb/storage/databasemdb.h>
#include <litedb/storage/index.h>
#include <litedb/catalog/sys_class.h>
#include <litedb/catalog/sys_index.h>
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
  rel->rd_rel.relkind = RELKIND_RELATION;
  rel->rd_rel.relid = id;
  return rel;
}

RelationPtr Relation::OpenTable(TransactionPtr tran, u64 id) {
  std::string name = std::to_string(id);
  KVStore* table = tran->Open(name, 0);
  KVStoreMdb* mdb = static_cast<KVStoreMdb*>(table);
  if (!mdb->SetCompare()) {
    mdb->SetCompare(u64_cmp);
  }
  RelationPtr rel(new Relation(table));
  rel->rd_rel.relid = id;
  rel->rd_rel.relkind = RELKIND_RELATION;

  if (!SysClass::GetCatalog(tran, id, &rel->rd_rel)) {
    return rel;
  }

  if (rel->rd_rel.relhasindex) {
    SysIndex::GetIndexList(tran, id, rel->rd_index);
  }
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
  rel->rd_rel.relid = id;
  rel->rd_rel.relkind = RELKIND_INDEX;
  return rel;
}

Relation::Relation(KVStore* table) :
    kvstore(table) {
  memset(&rd_rel, 0, sizeof(rd_rel));
}

void Relation::TableInsert(u64 id, const Tuple& tuple) {
  assert(rd_rel.relkind == RELKIND_RELATION);
  Slice key((char*) &id, sizeof(u64));
  Slice value;
  tuple.GetTupleData(value);
  kvstore->Put(key, value, 0);
}

i64 Relation::TableNextID() {
  assert(rd_rel.relkind == RELKIND_RELATION);
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
  assert(rd_rel.relkind == RELKIND_RELATION);
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
  assert(rel->rd_rel.relkind == RELKIND_RELATION);
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
  assert(scan->rel->rd_rel.relkind == RELKIND_RELATION);
  Slice key;
  Slice value;
  while (scan->cursor->Get(key, value, MDB_NEXT)) {
    u64 rowID = *(u64*) key.data();
    bool matched = true;

    for (int i = 0; i < scan->nkeys; ++i) {
      ScanKey* scanKey = scan->scanKey + i;

      Tuple tuple((char*) value.data(), value.size());
      tuple.SetRowID(rowID);

      if (scanKey->attno == 0 && scanKey->type != INT8OID) {
          elog(ERROR, "invalid key type %d", scanKey->type);
      }

      Slice column;
      TupleMeta entry;
      tuple.GetTupleMeta(scanKey->attno, entry);
      if (entry.type != scanKey->type) {
          elog(ERROR, "type not matched %d, %d", entry.type, scanKey->type);
      }
      column.assign(entry.data, entry.size);

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
  assert(scan->rel->rd_rel.relkind == RELKIND_RELATION);
  scan->rel->kvstore->Close(scan->cursor);
  scan = nullptr;
}

class SysScanDesc {
 public:
  RelationPtr tableRel;          /* catalog being scanned */
  RelationPtr indexRel;          /* NULL if doing table scan */
  TableScanDescPtr relScan;       /* only valid in storage-scan case */
  IndexScanDescPtr indexScan;     /* only valid in index-scan case */
};

SysScanDescPtr SysTableBeginScan(TransactionPtr txn,
                                 RelationPtr tableRel,
                                 i64 indexId,
                                 ScanKey* scanKey, int nkeys) {
  SysScanDescPtr scan(new SysScanDesc());
  scan->tableRel = tableRel;

  if (indexId) {
    scan->indexRel = Relation::OpenIndex(txn, indexId);
    SysIndex indexTup;
    if (!SysIndex::GetIndexTuple(txn, indexId, indexTup)) {
      elog(ERROR, "no such index %lu", indexId);
    }

    std::vector<ScanKey> indexKeys(nkeys);

    /* Change attribute numbers to be index column numbers. */
    for (int i = 0; i < nkeys; ++i) {
      ScanKey* key = scanKey + i;

      size_t j = 0;
      for (j = 0; j < indexTup.indkey.element_num; ++j) {
        i16 attno = VectorGet<i16>(&indexTup.indkey, j);
        if (key->attno == attno) {
          if (key->type != indexTup.indkey.element_type) {
            elog(ERROR, "type not matched");
          }
          ScanKey::Init(&indexKeys[i], attno, key->strategy, key->type, key->argument);
          break;
        }
      }

      if (j == indexTup.indkey.element_num) {
        elog(ERROR, "column is not in index");
      }
    }

    scan->indexScan = IndexBeginScan(tableRel, scan->indexRel, indexKeys.data(), indexKeys.size());
  } else {
    scan->relScan = TableBeginScan(tableRel, scanKey, nkeys);
  }
  return scan;
}

TuplePtr SysTableGetNext(SysScanDescPtr scan) {
  if (scan->indexRel) {
    return IndexGetNext(scan->indexScan);
  } else {
    return TableGetNext(scan->relScan);
  }
}

void SysTableEndScan(SysScanDescPtr& scan) {
  if (scan->indexScan) {
    IndexEndScan(scan->indexScan);
  } else {
    TableEndScan(scan->relScan);
  }
}

}