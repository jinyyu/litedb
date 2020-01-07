#ifndef LITEDB_LITEDB_PARSER_PLANNODES_H_
#define LITEDB_LITEDB_PARSER_PLANNODES_H_
#include <litedb/parser/nodes.h>

namespace db {

struct PlannedStmt {
  NodeTag type;
  CmdType commandType; /* select|insert|update|delete|utility */
  Node* utilityStmt;   /* non-null if this is utility stmt */
};

}

#endif //LITEDB_LITEDB_PARSER_PLANNODES_H_
