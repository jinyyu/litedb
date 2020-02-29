#include <litedb/parser/parse_target.h>
#include <assert.h>
#include <litedb/utils/elog.h>
#include <litedb/storage/relation.h>
#include <litedb/nodes/execnodes.h>

namespace db {

static List* ExpandColumnRefStar(ParseState* pstate, ColumnRef* cref, bool make_target_entry);
static List* ExpandAllTables(ParseState* pstate);

List* ExpandAllTables(ParseState* pstate) {
  List* targetList = nullptr;
  for (RangeTblEntry* entry: pstate->rtable) {

    switch (entry->rteKind) {
      case RTE_RELATION: {
        Relation* rel = Relation::OpenTable(CurrentTransaction, entry->relid);
        for (SysAttribute& attr : rel->rd_attr) {
          Var* var = makeVar(attr.attnum, attr.atttypid);

          targetList = lappend(targetList,
                               makeTargetEntry((Expr*) var, pstate->p_next_resno++, NameStr(&attr.attname)));
        }
        break;
      }
      default: {
        elog(ERROR, "invalid rtekind %d", entry->rteKind);
        break;
      }
    }
  }
  return targetList;
}

List* ExpandColumnRefStar(ParseState* pstate, ColumnRef* cref, bool make_target_entry) {
  return NULL;
}

List* TransformTargetList(ParseState* pstate, List* targetlist) {
  ListCell* o_target;
  List* p_target = NULL;
  foreach(o_target, targetlist) {
    ResTarget* res = (ResTarget*) lfirst(o_target);
    assert(res->type == T_ResTarget);

    if (res->val->type == T_A_Star) {
      /*
       * Target item is a bare '*', expand all tables
       *
       * (e.g., SELECT * FROM emp, dept)
       */

      return ExpandAllTables(pstate);

    } else if (res->val->type == T_ColumnRef) {
      fprintf(stderr, "------------ colun ref\n");
    } else {
      fprintf(stderr, "now now type\n");
    }

  }

  return p_target;
}

}
