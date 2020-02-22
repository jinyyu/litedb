#ifndef LITEDB_COMMAND_INDEXCMD_H_
#define LITEDB_COMMAND_INDEXCMD_H_
#include <litedb/int.h>
#include <litedb/nodes/execnodes.h>
#include <litedb/storage/database.h>

namespace db {

void IndexRegister(i64 table, i64 index, bool primaryKey, IndexInfo* indexInfo);
void BuildIndices(TransactionPtr txn);

}

#endif //LITEDB_COMMAND_INDEXCMD_H_
