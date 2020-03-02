#ifndef LITEDB_STORAGE_INDEX_H_
#define LITEDB_STORAGE_INDEX_H_
#include <litedb/storage/index.h>
#include <litedb/storage/relation.h>
#include <litedb/nodes/execnodes.h>

namespace db {


class IndexScanDesc;
typedef std::shared_ptr<IndexScanDesc> IndexScanDescPtr;

void IndexAmBuild(Relation* tableRel, Relation* index, IndexInfo* info);

void IndexAmInsert(Relation* index, TuplePtr tuple, IndexInfo* info);

IndexScanDescPtr IndexBeginScan(Relation* tableRel, Relation* index,
                                ScanKey* scanKey, int nkeys);

TuplePtr IndexGetNext(IndexScanDescPtr desc);

void IndexEndScan(IndexScanDescPtr& scan);

}

#endif //LITEDB_STORAGE_INDEX_H_
