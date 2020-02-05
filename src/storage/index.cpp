#include <litedb/storage/index.h>
#include <litedb/utils/elog.h>
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
  Table* table = index->GetTable();
  assert(tuple->ContainsRowID());
  u64 rowID = tuple->GetRowID();
  Slice key;
  Slice value;
  std::vector<TupleMeta> columns(info->ii_NumIndexKeyAttrs);

  for (int i = 0; i < info->ii_NumIndexKeyAttrs; ++i) {
    i16 column = info->ii_IndexAttrNumbers[i];
    if (column == 0) {

      columns[i].type = INT8OID;
      columns[i].data = (char*) &rowID;
      columns[i].size = sizeof(rowID);

    } else {
      tuple->Get(column - 1, columns[i]);
    }
  }

  TuplePtr indexTuple = Tuple::Construct(columns);
  Slice tupleData;
  indexTuple->GetTupleData(tupleData);
  int flags = 0;

  if (info->ii_Unique) {
    if (table->Get(tupleData, value)) {
      elog(ERROR, "duplicate key value");
    }
  }
  value.assign((char*) &rowID, sizeof(rowID));
  table->Put(key, value, flags);
}

class IndexScanDesc {
 public:
  IndexScanDesc()
      : tableRel(nullptr),
        indexRel(nullptr),
        numberOfKeys(0),
        numberOfOrderBys(0),
        keyData(nullptr),
        orderByData(nullptr),
        cursor(nullptr),
        commonKey(0),
        scanFlag(0),
        finished(false) {

  }

  ~IndexScanDesc() {
    if (keyData) {
      free(keyData);
    }
    if (orderByData) {
      free(orderByData);
    }
    if (cursor) {
      indexRel->GetTable()->Close(cursor);
    }
  }

  Relation* tableRel;
  Relation* indexRel;
  int numberOfKeys;         /* number of index qualifier conditions */
  int numberOfOrderBys;     /* number of ordering operators */
  ScanKey* keyData;          /* array of index qualifier descriptors */
  ScanKey* orderByData;      /* array of ordering op descriptors */
  Cursor* cursor;
  int commonKey;
  StrategyNumber commonStrategy{};
  TuplePtr indexTuple;
  TuplePtr tuple;
  int scanFlag;
  bool finished;
};

IndexScanDescPtr IndexBeginScan(RelationPtr tableRel, RelationPtr index,
                                ScanKey* scanKey, int nkeys,
                                ScanKey* orderbys, int norderbys) {
  IndexScanDescPtr desc(new IndexScanDesc());
  desc->tableRel = tableRel.get();
  desc->indexRel = index.get();
  if (scanKey && nkeys) {
    size_t size = sizeof(ScanKey) * nkeys;
    desc->keyData = (ScanKey*) malloc(size);
    memcpy(desc->keyData, scanKey, size);
  }

  if (orderbys && norderbys) {
    size_t size = sizeof(ScanKey) * norderbys;
    desc->orderByData = (ScanKey*) malloc(size);
    memcpy(desc->orderByData, orderbys, size);
  }

  desc->cursor = index->GetTable()->Open();

  assert(desc->numberOfKeys > 0);
  desc->commonKey = 1;
  desc->commonStrategy = scanKey[1].strategy;
  for (int i = 2; i <= desc->numberOfKeys; ++i) {
    StrategyNumber strategy = desc->keyData[i].strategy;
    if (strategy != desc->commonStrategy) {
      break;
    }
    desc->commonKey = i;
    desc->commonStrategy = strategy;
  }

  std::vector<TupleMeta> commonIndex(desc->commonKey);
  for (int i = 0; i < desc->commonKey; ++i) {
    ScanKey& key = desc->keyData[i];
    commonIndex[i] = TupleMeta(key.type, key.argument.data(), key.argument.size());
  }

  TuplePtr indexTuple = Tuple::Construct(commonIndex);
  Slice tupleData;
  Slice indexVal;
  indexTuple->GetTupleData(tupleData);

  //Position at first key greater than or equal to specified key
  bool isSeek = desc->cursor->Get(tupleData, indexVal, MDB_SET_RANGE);

  switch (desc->commonStrategy) {
    case BTLessStrategyNumber:
    case BTLessEqualStrategyNumber: {
      if (!isSeek) {
        desc->finished = true;
      }
      desc->scanFlag = MDB_PREV_DUP;
      break;
    }
    case BTEqualStrategyNumber:
    case BTGreaterStrategyNumber:
    case BTGreaterEqualStrategyNumber: {
      if (!isSeek) {
        desc->finished = true;
      }
      desc->scanFlag = MDB_NEXT_DUP;
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

  Slice key;
  Slice value;
  while (!desc->cursor->Get(key, value, desc->scanFlag)) {
    assert(value.size() == sizeof(i64));
    i64 rowID = *(i64*) value.data();
    Slice tupleData;
    if (!desc->tableRel->GetTable()->Get(value, tupleData)) {
      elog(ERROR, "get tuple error %d", rowID);
      desc->finished = true;
      break;
    }

    desc->tuple = std::make_shared<Tuple>((char*) tupleData.data(), tupleData.size());

    break;
  }

  if (desc->finished) {
    return nullptr;
  } else {
    return desc->tuple;
  }
}
void IndexEndScan(IndexScanDescPtr& scan) {
  scan = nullptr;
}

}
