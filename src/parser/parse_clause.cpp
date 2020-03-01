#include <litedb/parser/parse_clause.h>
#include <litedb/utils/elog.h>
#include <litedb/catalog/sys_class.h>
#include <unordered_set>
namespace db {

static RangeTblRef* TransformFromClauseItem(ParseState* pstate, Node* node, RangeTblEntry** top_rte, int* top_rtindex);
static void CheckNameSpaceConflicts(ParseState* pstate);

void CheckNameSpaceConflicts(ParseState* pstate) {
  std::unordered_set<std::string> names;
  ListCell* cell;
  foreach(cell, pstate->rtable) {
    RangeTblEntry* rte = (RangeTblEntry*) lfirst(cell);

    if (!rte->alias) {
      continue;
    }
    if (names.find(rte->alias) != names.end()) {
      elog(ERROR, "table name \"%s\" specified more than once", rte->alias);
    }
    names.insert(rte->alias);
  }
}

RangeTblRef* TransformFromClauseItem(ParseState* pstate, Node* node, RangeTblEntry** top_rte, int* top_rtindex) {
  switch (node->type) {
    case T_RangeVar: {
      RangeVar* rv = (RangeVar*) node;
      TuplePtr tuple = SysClass::GetSysClass(CurrentTransaction, rv->relname);
      if (!tuple) {
        elog(ERROR, " relation \"%s\" does not exist", rv->relname);
      }

      SysClass sys_class;
      SysClass::FromTuple(*tuple, sys_class);

      RangeTblEntry* rte = makeNode(RangeTblEntry);
      RangeTblRef* rtr = makeNode(RangeTblRef);

      /* assume new rte is at end */
      int rtindex = list_length(pstate->rtable);

      rte->relkind = sys_class.relkind;
      rte->rteKind = RTE_RELATION;
      rte->relid = sys_class.relid;

      if (rv->alias) {
        rte->alias = rv->alias;
      } else {
        rte->alias = rv->relname;
      }
      pstate->rtable = lappend(pstate->rtable, rte);

      rtr->rtindex = rtindex;

      *top_rte = rte;
      *top_rtindex = rtindex;

      return rtr;
    }
    default: {
      elog(ERROR, "unrecognized node type: %d", node->type);
      break;
    }
  }

}

void TransformFromClause(ParseState* pstate, List* fromClause) {
  ListCell* cell;
  foreach (cell, fromClause) {
    Node* node = (Node*) lfirst(cell);
    RangeTblEntry* rte;
    int rtindex;
    RangeTblRef* rtr = TransformFromClauseItem(pstate, node, &rte, &rtindex);

    CheckNameSpaceConflicts(pstate);
    pstate->joinlist = lappend(pstate->joinlist, rtr);
  }
}

/*
 * transformWhereClause -
 *	  Transform the qualification and make sure it is of type boolean.
 *	  Used for WHERE and allied clauses.
 */
Node* TransformWhereClause(ParseState* pstate, Node* clause) {
  return NULL;
}

}