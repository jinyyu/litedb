#include <litedb/storage/scan_key.h>
#include <litedb/utils/elog.h>

namespace db {

bool ScanKey::PerformCompare(ScanKey* self, TypeCmpCallback* cmp, const Slice& column) {
  Entry entryArg;
  entryArg.data = (void*) self->argument.data();
  entryArg.size = self->argument.size();

  Entry entryColumn;
  entryColumn.data = (void*) column.data();
  entryColumn.size = column.size();

  int ret = cmp(&entryArg, &entryColumn);

  switch (self->strategy) {
    case BTLessStrategyNumber: {
      return ret < 0;
    }
    case BTLessEqualStrategyNumber: {
      return ret <= 0;
    }
    case BTEqualStrategyNumber: {
      return ret == 0;
    }
    case BTGreaterEqualStrategyNumber: {
      return ret >= 0;
    }
    case BTGreaterStrategyNumber: {
      return ret > 0;
    }
    default: {
      elog(ERROR, "invalid strategy %d", self->strategy);
    }
  }
  return false;
}

}

