#include <litedb/parser/analyze.h>
#include <litedb/parser/parse_clause.h>
#include <litedb/parser/parse_target.h>
#include <litedb/utils/elog.h>
#include <litedb/utils/env.h>
#include <litedb/nodes/execnodes.h>

namespace db {

Query* ParseAnalyze(Node* parseTree, const char* queryString) {
  DisplayParseNode(parseTree);
  Query* query;
  ParseState* pstate = new ParseState();
  pstate->sourceText = queryString;

  query = TransformStmt(pstate, parseTree);

  SessionEnv->Drop(pstate);
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
  TransformFromClause(pstate, stmt->fromClause);

  query->targetList = TransformTargetList(pstate, stmt->targetList);

  Node* qual = TransformWhereClause(pstate, stmt->whereClause);

  query->rtable = pstate->rtable;
  query->jointree = makeFromExpr(pstate->joinlist, qual);

  return query;
}

}