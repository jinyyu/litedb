#include <litedb/storage/relation.h>
#include <litedb/storage/databasemdb.h>
#include <litedb/storage/index.h>
#include <litedb/catalog/sys_class.h>
#include <litedb/catalog/sys_index.h>
#include <lmdb.h>
#include <litedb/utils/elog.h>
#include <assert.h>

namespace db {

static TuplePtr RowidScanTableGetNext(TableScanDescPtr scan);
static void RowidScanTableBeginScan(TableScanDescPtr desc);

class TableScanDesc {
 public:
  TableScanDesc()
      : cursor(nullptr),
        scanKey(nullptr),
        nkeys(0),
        totalFetch(0),
        rowidScan(false),
        finished(false) {

  }

  ~TableScanDesc() {
    if (scanKey) {
      free(scanKey);
    }
  }

  RelationPtr rel;    /* the relation of table */
  Cursor* cursor;     /* the scan cursor */
  ScanKey* scanKey;   /* scan key */
  int nkeys;          /* number of scan key */
  u64 totalFetch;     /* total number of tuples have been fetched*/
  bool rowidScan;     /* 可以使用rowid使用索引扫描 ? */
  bool finished;      /* 扫描结束 */
  TuplePtr lastTuple;
};

RelationPtr Relation::Create(TransactionPtr txn, i64 id) {
  std::string name = std::to_string(id);
  KVStore* table = txn->Open(name, MDB_CREATE);
  KVStoreMdb* mdb = static_cast<KVStoreMdb*>(table);
  if (!mdb->SetCompare()) {
    mdb->SetCompare(u64_cmp);
  }
  RelationPtr rel(new Relation(table));
  rel->rd_rel.relkind = RELKIND_RELATION;
  rel->rd_rel.relid = id;
  return rel;
}

RelationPtr Relation::OpenTable(TransactionPtr txn, i64 id) {
  RelationPtr rel;
  rel = txn->GetOpenRelation(id);
  if (rel) {
    return rel;
  }

  std::string name = std::to_string(id);
  KVStore* table = txn->Open(name, 0);
  KVStoreMdb* mdb = static_cast<KVStoreMdb*>(table);
  if (!mdb->SetCompare()) {
    mdb->SetCompare(u64_cmp);
  }
  rel = std::make_shared<Relation>(table);
  rel->rd_rel.relid = id;
  rel->rd_rel.relkind = RELKIND_RELATION;

  txn->InsertOpenRelation(id, rel);

  if (!SysClass::GetCatalog(txn, id, &rel->rd_rel)) {
    return rel;
  }

  if (rel->rd_rel.relhasindex) {
    SysIndex::GetIndexList(txn, id, rel->rd_index);
  }
  return rel;
}

RelationPtr Relation::OpenIndex(TransactionPtr txn, i64 id) {
  RelationPtr rel;
  rel = txn->GetOpenRelation(id);
  if (rel) {
    return rel;
  }

  std::string name = std::to_string(id);
  KVStore* table = txn->Open(name, MDB_CREATE | MDB_DUPSORT);
  KVStoreMdb* mdb = static_cast<KVStoreMdb*>(table);
  if (!mdb->SetCompare()) {
    mdb->SetCompare(index_cmp);
  }
  rel = std::make_shared<Relation>(table);
  rel->rd_rel.relid = id;
  rel->rd_rel.relkind = RELKIND_INDEX;

  txn->InsertOpenRelation(id, rel);

  return rel;
}

Relation::Relation(KVStore* table) :
    kvstore(table) {
  memset(&rd_rel, 0, sizeof(rd_rel));
}

void Relation::TableInsert(i64 id, const Tuple& tuple) {
  assert(rd_rel.relkind == RELKIND_RELATION);
  Slice key(&id);
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

void RowidScanTableBeginScan(TableScanDescPtr desc) {
  assert(desc->scanKey->type == INT8OID);
  assert(desc->nkeys == 1);

  desc->rowidScan = true;
  ScanKey* scanKey = desc->scanKey;

  Slice value;
  i64 rowid = *(i64*) scanKey->argument.data();

  if (scanKey->strategy == BTEqualStrategyNumber) {
    desc->finished = true;

    if (desc->rel->kvstore->Get(scanKey->argument, value)) {
      desc->lastTuple = std::make_shared<Tuple>((char*) value.data(), value.size());
      desc->lastTuple->SetRowID(rowid);
    }
  } else if (scanKey->strategy == BTGreaterEqualStrategyNumber || scanKey->strategy == BTGreaterStrategyNumber) {
    desc->cursor = desc->rel->kvstore->Open();
    //Position at first key greater than or equal to specified key
    Slice key = scanKey->argument;
    bool ok = desc->cursor->Get(key, value, MDB_SET_RANGE);
    if (!ok) {
      desc->finished = true;
      return;
    }

    if (ScanKey::CheckSatisfy(scanKey, key)) {
      desc->lastTuple = std::make_shared<Tuple>((char*) value.data(), value.size());
      desc->lastTuple->SetRowID(rowid);
    }
  } else {
    desc->cursor = desc->rel->kvstore->Open();
  }
}

TableScanDescPtr TableBeginScan(RelationPtr rel, ScanKey* scanKey, int nkeys) {
  assert(rel->rd_rel.relkind == RELKIND_RELATION);
  TableScanDescPtr desc(new TableScanDesc());
  desc->rel = rel;
  if (scanKey && nkeys) {
    desc->nkeys = nkeys;
    size_t bytes = sizeof(ScanKey) * nkeys;
    desc->scanKey = (ScanKey*) malloc(bytes);
    memcpy(desc->scanKey, scanKey, bytes);
  }

  if (nkeys == 1 && scanKey->attno == 0) {
    RowidScanTableBeginScan(desc);
  } else {
    desc->cursor = rel->kvstore->Open();
  }
  return desc;
}

TuplePtr TableGetNext(TableScanDescPtr scan) {
  if (scan->lastTuple) {
    TuplePtr tuple = scan->lastTuple;
    scan->lastTuple = nullptr;
    return tuple;
  }

  if (scan->finished) {
    return nullptr;
  }

  if (scan->rowidScan) {
    return RowidScanTableGetNext(scan);
  }

  //seq scan
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

TuplePtr RowidScanTableGetNext(TableScanDescPtr scan) {
  TuplePtr tuple;
  Slice key;
  Slice value;
  assert(scan->scanKey->attno == 0 && scan->nkeys == 1);
  i64 rowid = 0;

  switch (scan->scanKey->strategy) {
    case BTLessStrategyNumber:
    case BTLessEqualStrategyNumber: {
      TuplePtr tup;
      bool satisfy;
      bool ok = scan->cursor->Get(key, value, MDB_NEXT);

      if (!ok) {
        scan->finished = true;
        break;
      }
      rowid = *(i64*) key.data();
      tup = std::make_shared<Tuple>((char*) value.data(), value.size());

      satisfy = ScanKey::CheckSatisfy(scan->scanKey, key);
      if (!satisfy) {
        scan->finished = true;
        break;
      }

      tup->SetRowID(rowid);

      if (scan->scanKey->strategy == BTEqualStrategyNumber) {
        scan->finished = true;
      }
      tuple = tup;
      break;
    }

    case BTGreaterEqualStrategyNumber:
    case BTGreaterStrategyNumber: {
      bool ok = scan->cursor->Get(key, value, MDB_NEXT);
      if (!ok) {
        scan->finished = true;
        break;
      }

      tuple = std::make_shared<Tuple>((char*) value.data(), value.size());
      tuple->SetRowID(*(i64*) key.data());
      break;
    }
    default: {
      elog(ERROR, "invalid strategy %d", scan->scanKey->strategy);
      break;
    }
  }

  return tuple;
}

void TableEndScan(TableScanDescPtr& scan) {
  assert(scan->rel->rd_rel.relkind == RELKIND_RELATION);
  if (scan->cursor) {
    scan->rel->kvstore->Close(scan->cursor);
  }
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

  if ((scanKey == nullptr || nkeys == 0)
      || (nkeys == 1 && scanKey->attno == 0)) {
    scan->relScan = TableBeginScan(tableRel, scanKey, nkeys);
    return scan;
  }

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
      for (j = 0; j < static_cast<size_t>(indexTup.indnatts); ++j) {
        i16 attno = indexTup.indkey[j];
        if (key->attno == attno) {
          ScanKey::Init(&indexKeys[i], attno, key->strategy, key->type, key->argument);
          break;
        }
      }

      if (j == static_cast<size_t>(indexTup.indnatts)) {
        elog(ERROR, "column is not in index");
      }
    }

    scan->indexScan = IndexBeginScan(tableRel, scan->indexRel, indexKeys.data(), indexKeys.size());
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