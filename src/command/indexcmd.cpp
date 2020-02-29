#include <litedb/command/indexcmd.h>
#include <vector>
#include <litedb/utils/env.h>
#include <litedb/catalog/catalog.h>
#include <litedb/catalog/sys_index.h>
#include <litedb/storage/relation.h>
#include <litedb/storage/index.h>

namespace db {

struct IndexRegisterInfo {
  i64 table;
  i64 index;
  bool primaryKey;
  IndexInfo info;
};

static std::vector<IndexRegisterInfo*> IndexRegisterInfoList;

void IndexRegister(i64 table, i64 index, bool primaryKey, IndexInfo* indexInfo) {
  IndexRegisterInfo* info = (IndexRegisterInfo*) SessionEnv->Malloc(sizeof(IndexRegisterInfo));
  info->table = table;
  info->index = index;
  info->primaryKey = primaryKey;
  memcpy(&info->info, indexInfo, sizeof(IndexInfo));
  if (info->primaryKey) {
    assert(indexInfo->ii_Unique);
  }

  IndexRegisterInfoList.push_back(info);
}

void BuildIndices(TransactionPtr txn) {

  for (IndexRegisterInfo* info: IndexRegisterInfoList) {
    RelationPtr table = Relation::Create(txn, info->table);
    Relation* index = Relation::OpenIndex(txn, info->index);

    SysIndex sys_index;
    memset(&sys_index, 0, sizeof(sys_index));

    sys_index.indexrelid = info->index;
    sys_index.indrelid = info->table;
    sys_index.indnatts = info->info.ii_NumIndexKeyAttrs;
    sys_index.indisunique = info->info.ii_Unique;
    sys_index.indisprimary = info->primaryKey;
    memcpy(sys_index.indkey, info->info.ii_IndexAttrNumbers, sizeof(sys_index.indkey));

    if (sys_index.indexrelid )
    SysIndex::CreateEntry(txn, sys_index);
    IndexAmBuild(table.get(), index, &info->info);

    SessionEnv->Free(info);
  }
  IndexRegisterInfoList.clear();
}

}
