#ifndef LITEDB_COMMAND_INDEXCMD_H_
#define LITEDB_COMMAND_INDEXCMD_H_
#include <litedb/int.h>
#include <litedb/nodes/execnodes.h>

namespace db {

void IndexRegister(i64 table, i64 index, IndexInfo* indexInfo);
void BuildIndices();

}

#endif //LITEDB_COMMAND_INDEXCMD_H_
