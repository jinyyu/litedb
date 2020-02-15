#include <litedb/storage/index.h>
#include <litedb/utils/elog.h>
#include <litedb/catalog/sys_class.h>
#include <stdlib.h>

#include <memory>

namespace db {

void IndexAmBuild(RelationPtr tableRel, RelationPtr index, IndexInfo* info) {
  TableScanDescPtr scan = TableBeginScan(tableRel, nullptr, 0);
  TuplePtr tuple;
  while ((tuple = TableGetNext(scan)) != nullptr) {
    IndexAmInsert(index, tuple, info);
  }
  TableEndScan(scan);
}

void IndexAmInsert(RelationPtr index, TuplePtr tuple, IndexInfo* info) {
  assert(tuple->ContainsRowID());
  u64 rowID = tuple->GetRowID();
  Slice value;
  std::vector<TupleMeta> columns(info->ii_NumIndexKeyAttrs);

  for (int i = 0; i < info->ii_NumIndexKeyAttrs; ++i) {
    i16 column = info->ii_IndexAttrNumbers[i];
    tuple->GetTupleMeta(column, columns[i]);
  }

  TuplePtr indexTuple = Tuple::Construct(rowID, columns);
  Slice tupleData;
  indexTuple->GetTupleData(tupleData);
  int flags = 0;

  if (info->ii_Unique) {
    if (index->kvstore->Get(tupleData, value)) {
      elog(ERROR, "duplicate key value");
    }
  }
  value.assign((char*) &rowID, sizeof(rowID));
  assert(index->rd_rel.relkind == RELKIND_INDEX);
  index->kvstore->Put(tupleData, value, flags);
}

class IndexScanDesc {
 public:
  IndexScanDesc()
      : numberOfKeys(0),
        keyData(nullptr),
        cursor(nullptr),
        commonKeys(0),
        commonStrategy(0),
        scanFlag((MDB_cursor_op) 0),
        finished(false),
        rowID(0),
        firstIndexTuple(false) {

  }

  ~IndexScanDesc() {
    if (keyData) {
      free(keyData);
    }
  }

  bool FetchNextIndexTuple() {
    bool fetched;
    if (!firstIndexTuple) {
      Slice key;
      Slice value;
      fetched = cursor->Get(key, value, scanFlag);
      if (fetched) {
        rowID = *(i64*) value.data();
        indexTuple = std::make_shared<Tuple>((char*) key.data(), key.size());
      } else {
        finished = true;
      }
    } else {
      fetched = true;
      assert(rowID);
    }

    firstIndexTuple = false;
    return fetched;
  }

  bool IsIndexTupleSatisfyCommonStrategy() {
    assert(indexTuple);
    for (int i = 0; i < commonKeys; ++i) {
      TupleMeta meta;
      indexTuple->GetTupleMeta(i + 1, meta); //index没有rowid
      assert(meta.type = keyData[i].type);
      Slice column(meta.data, meta.size);
      if (ScanKey::PerformCompare(keyData + i, column) != 0) {
        return false;
      }
    }
    return true;
  }

  void GetTableTuple() {
    assert(rowID);
    Slice key((char*) &rowID, sizeof(rowID));
    Slice value;
    bool ok = tableRel->kvstore->Get(key, value);
    if (!ok) {
      elog(ERROR, "no such tuple error %lu", rowID);
    }
    tuple = std::make_shared<Tuple>((char*) value.data(), value.size());
  }

  RelationPtr tableRel;
  RelationPtr indexRel;
  int numberOfKeys;         /* number of index qualifier conditions */
  ScanKey* keyData;         /* array of index qualifier descriptors */
  Cursor* cursor;
  int commonKeys;
  StrategyNumber commonStrategy;
  TuplePtr tuple;
  MDB_cursor_op scanFlag;
  bool finished;            /* is scan finished? */

  TuplePtr indexTuple;
  i64 rowID;
  bool firstIndexTuple;          /**/
};

IndexScanDescPtr IndexBeginScan(RelationPtr tableRel, RelationPtr index,
                                ScanKey* scanKey, int nkeys) {
  assert(index->rd_rel.relkind == RELKIND_INDEX);
  assert(nkeys > 0);
  assert(scanKey);

  IndexScanDescPtr desc(new IndexScanDesc());
  desc->tableRel = tableRel;
  desc->indexRel = index;
  desc->numberOfKeys = nkeys;

  desc->keyData = (ScanKey*) malloc(sizeof(ScanKey) * nkeys);
  memcpy(desc->keyData, scanKey, sizeof(ScanKey) * nkeys);
  desc->cursor = index->kvstore->Open();

  desc->commonKeys = 1;
  desc->commonStrategy = scanKey[0].strategy;
  for (int i = 2; i <= desc->numberOfKeys; ++i) {
    StrategyNumber strategy = desc->keyData[i].strategy;
    if (strategy != desc->commonStrategy) {
      break;
    }
    desc->commonKeys = i;
    desc->commonStrategy = strategy;
  }

  std::vector<TupleMeta> commonIndex(desc->commonKeys);
  for (int i = 0; i < desc->commonKeys; ++i) {
    ScanKey& key = desc->keyData[i];
    commonIndex[i] = TupleMeta(key.type, key.argument.data(), key.argument.size());
  }

  TuplePtr indexTuple = Tuple::Construct(0, commonIndex);
  Slice tupleData;
  Slice indexVal;
  indexTuple->GetTupleData(tupleData);

  //Position at first key greater than or equal to specified key
  bool seek = desc->cursor->Get(tupleData, indexVal, MDB_SET_RANGE);

  switch (desc->commonStrategy) {
    case BTLessStrategyNumber:
    case BTLessEqualStrategyNumber: {
      elog(ERROR, "not supported Strategy %d", desc->commonStrategy);
      break;
    }
    case BTEqualStrategyNumber: {
      if (seek) {
        desc->firstIndexTuple = true;
        desc->scanFlag = MDB_NEXT;
        assert(indexVal.size() == sizeof(i64));
        desc->rowID = *(i64*) indexVal.data();
        desc->indexTuple = std::make_shared<Tuple>((char*) tupleData.data(), tupleData.size());
      } else {
        desc->finished = true;
      }
      break;
    }
    case BTGreaterEqualStrategyNumber:
    case BTGreaterStrategyNumber: {
      elog(ERROR, "not supported Strategy %d", desc->commonStrategy);
      break;
    }
    default: {
      elog(ERROR, "invalid strategy %d", desc->commonStrategy);
      break;
    }
  }

  return desc;
}

TuplePtr IndexGetNext(IndexScanDescPtr desc) {
  if (desc->finished) {
    return nullptr;
  }

  while (desc->FetchNextIndexTuple()) {
    switch (desc->commonStrategy) {
      case BTLessStrategyNumber:
      case BTLessEqualStrategyNumber: {
        elog(ERROR, "not supported Strategy %d", desc->commonStrategy);
        break;
      }
      case BTEqualStrategyNumber: {
        //前缀只要有一个不匹配，扫描结束
        if (!desc->IsIndexTupleSatisfyCommonStrategy()) {
          desc->finished = true;
          goto fetchFinished;
        }

        for (int i = desc->commonKeys; i < desc->numberOfKeys; ++i) {
          TupleMeta meta;
          desc->indexTuple->GetTupleMeta(i, meta);
          Slice column(meta.data, meta.size);

          if (!ScanKey::PerformCompare(desc->keyData + i, column)) {
            //有一个不匹配，继续fetch
            goto continueFetchTuple;
          }
        }

        //all index keys satisfy
        desc->GetTableTuple();
        goto fetchFinished;
      }
      case BTGreaterEqualStrategyNumber:
      case BTGreaterStrategyNumber: {
        elog(ERROR, "not supported Strategy %d", desc->commonStrategy);
        break;
      }
      default: {
        elog(ERROR, "invalid strategy %d", desc->commonStrategy);
        break;
      }
    }

continueFetchTuple:
    {}
  }

fetchFinished:
  if (desc->finished) {
    return nullptr;
  } else {
    return desc->tuple;
  }
}

void IndexEndScan(IndexScanDescPtr& scan) {
  if (scan->cursor) {
    scan->indexRel->kvstore->Close(scan->cursor);
  }
  scan = nullptr;
}

}
