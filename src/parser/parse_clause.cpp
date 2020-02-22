#include <litedb/parser/parse_clause.h>
#include <litedb/utils/elog.h>
#include <litedb/catalog/sys_class.h>
#include <litedb/catalog/sys_class.h>
namespace db {

static Node* TransformFromClauseItem(ParseState* pstate, Node* node, RangeTblEntry** rte, int* rtindex);

Node* TransformFromClauseItem(ParseState* pstate, Node* node, RangeTblEntry** rte, int* rtindex) {
  switch (node->type) {
    case T_RangeVar: {
      RangeVar* rv = (RangeVar*) node;
      RangeTblEntry* rte = makeNode(RangeTblEntry);



      break;
    }
    default: {
      elog(ERROR, "unrecognized node type: %d", node->type);
      break;
    }
  }

}

void TransformFromClause(ParseState* pstate, List<Node>* fromClause) {
  if (!fromClause) {
    return;
  }

  for (Node* node : fromClause->list) {
    RangeTblEntry* rte;
    int rtindex;
    TransformFromClauseItem(pstate, node, &rte, &rtindex);
  }
}

}