#include <litedb/command/indexcmd.h>
#include <vector>
#include <litedb/utils/env.h>

namespace db {

struct IndexRegisterInfo {
  i64 table;
  i64 index;
  IndexInfo info;
};

static std::vector<IndexRegisterInfo*> IndexRegisterInfoList;

void IndexRegister(i64 table, i64 index, IndexInfo* indexInfo) {
  IndexRegisterInfo* info = (IndexRegisterInfo*) SessionEnv->Malloc(sizeof(IndexRegisterInfo));
  info->table = table;
  info->index = index;
  memcpy(&info->info, indexInfo, sizeof(IndexRegisterInfo));

  IndexRegisterInfoList.push_back(info);
}

void BuildIndices() {
  for (IndexRegisterInfo* info: IndexRegisterInfoList) {
    SessionEnv->Free(info);
  }
  IndexRegisterInfoList.clear();
}

}
