#include <litedb/parser/analyze.h>
#include <litedb/utils/elog.h>
#include <litedb/utils/env.h>
namespace db {

ParseState* ParseState::Create() {
  ParseState* pstate = (ParseState*) SessionEnv->Malloc0(sizeof(ParseState));
  return pstate;
}

Query* ParseAnalyze(Node* parseTree, const char* queryString) {
  DisplayParseNode(parseTree);
  Query* result;
  ParseState* pstate = ParseState::Create();

  switch (parseTree->type) {
    case T_CreateTableStmt: {
      result = makeNode(Query);
      result->commandType = CMD_CMD_UTILITY;
      result->utilityStmt = parseTree;
      break;
    }

    default: {
      elog(ERROR, "invalid type %d", parseTree->type);
    }
  }

  SessionEnv->Free(pstate);
  return result;
}

}