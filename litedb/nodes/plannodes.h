#ifndef LITEDB_PARSER_PLANNODES_H_
#define ITEDB_PARSER_PLANNODES_H_
#include <litedb/nodes/parsenodes.h>

namespace db {

struct Plan {

};

struct PlannedStmt {
  NodeTag type;
  CmdType commandType; /* select|insert|update|delete|utility */
  Plan* planTree;       /* tree of Plan nodes */
  Node* utilityStmt;   /* non-null if this is utility stmt */
};

}

#endif
