#include <litedb/parser/analyze.h>
#include <litedb/parser/parse_clause.h>
#include <litedb/utils/elog.h>
#include <litedb/utils/env.h>
namespace db {

Query* ParseAnalyze(Node* parseTree, const char* queryString) {
  DisplayParseNode(parseTree);
  Query* query;
  ParseState* pstate = new ParseState();
  pstate->sourceText = queryString;

  query = TransformStmt(pstate, parseTree);

  delete(pstate);
  return query;
}

Query* TransformStmt(ParseState* pstate, Node* parseTree) {
  Query* result;
  switch (parseTree->type) {
    case T_CreateTableStmt: {
      result = TransformCreateTableStmt(pstate, (CreateTableStmt*) parseTree);
      break;
    }
    case T_SelectStmt: {
      result = TransformSelectStmt(pstate, (SelectStmt*) parseTree);
      break;
    }
    default: {
      elog(ERROR, "invalid stmt %d", parseTree->type);
    }
  }

  return result;
}

Query* TransformCreateTableStmt(ParseState* pstate, CreateTableStmt* stmt) {
  Query* query = makeNode(Query);
  query->commandType = CMD_CMD_UTILITY;
  query->utilityStmt = (Node*) stmt;
  return query;
}

Query* TransformSelectStmt(ParseState* pstate, SelectStmt* stmt) {
  Query* query = makeNode(Query);
  query->commandType = CMD_SELECT;

  /* process the FROM clause */
  db::TransformFromClause(pstate, stmt->fromClause);

  return query;
}

}