#ifndef LITEDB_STORAGE_SCAN_KEY_H_
#define LITEDB_STORAGE_SCAN_KEY_H_
#include <litedb/int.h>
#include <litedb/utils/slice.h>
#include <litedb/utils/compare.h>

namespace db {

typedef u16 StrategyNumber;

#define BTLessStrategyNumber            1
#define BTLessEqualStrategyNumber       2
#define BTEqualStrategyNumber           3
#define BTGreaterEqualStrategyNumber    4
#define BTGreaterStrategyNumber         5

#define BTMaxStrategyNumber             5

struct ScanKey {
  int flags;        /* flags */
  i16 attno;        /* table or index column number */
  StrategyNumber strategy; /* operator strategy number */
  u32 type;         /* type of the argument*/
  Slice argument;   /* argument to compare */

  static void Init(ScanKey* entry, i16 attno, StrategyNumber strategy, u32 typeID, const Slice& argument) {
    entry->flags = 0;
    entry->attno = attno;
    entry->strategy = strategy;
    entry->type = typeID;
    entry->argument = argument;
  }

  static bool PerformCompare(ScanKey* entry, TypeCmpCallback* cmp, const Slice& column);
};

}

#endif //LITEDB_STORAGE_SCAN_KEY_H_
