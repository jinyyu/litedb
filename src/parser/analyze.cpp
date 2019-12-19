#include <litedb/parser/analyze.h>
#include <litedb/utils/elog.h>

namespace db {

ParseState* ParseState::Create() {
  ParseState* pstate = (ParseState*) Malloc0(sizeof(ParseState));
  return pstate;
}

Query* ParseAnalyze(Node* parseTree, const char* queryString) {
  NodeDisplay(parseTree);
  Query* result;
  ParseState* pstate = ParseState::Create();

  switch (parseTree->type) {
    case T_CreateTableStmt: {
      result = makeNode(Query);
      result->commandType = CMD_CREATE;
      result->utilityStmt = parseTree;
      break;
    }

    default: {
      elog(ERROR, "invalid type %d", parseTree->type);
    }
  }

  Free(pstate);
  return result;
}

}