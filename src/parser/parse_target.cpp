#include <litedb/parser/parse_target.h>
#include <assert.h>
#include <litedb/utils/elog.h>
#include <litedb/storage/relation.h>
#include <litedb/nodes/execnodes.h>
#include <litedb/nodes/value.h>
#include <litedb/parser/parse_relation.h>

namespace db {

static List* ExpandAllTables(ParseState* pstate);
static List* TransformColumnRef(ParseState* pstate, ColumnRef* cref, char* columnName);
static List* ExpandSingleTable(ParseState* pstate, RangeTblEntry* rte);
static TargetEntry* TransformTargetEntry(ParseState* pstate, RangeTblEntry* rte, Node* node, char* colname);
/*
 * ExpandSingleTable()
 *		Transforms foo.* into a list of expressions or targetlist entries.
 */
List* ExpandSingleTable(ParseState* pstate, RangeTblEntry* rte) {
  List* targetList = nullptr;
  int sublevels_up;
  int rtindex = RTERangeTablePosn(pstate, rte, &sublevels_up);

  switch (rte->rteKind) {
    case RTE_RELATION: {
      Relation* rel = Relation::OpenTable(CurrentTransaction, rte->relid);
      for (SysAttribute& attr : rel->rd_attr) {
        Var* var = makeVar(rtindex, attr.attnum, attr.atttypid);

        targetList = lappend(targetList,
                             makeTargetEntry((Expr*) var, pstate->p_next_resno++, NameStr(&attr.attname)));
      }
      break;
    }
    default: {
      elog(ERROR, "invalid rtekind %d", rte->rteKind);
      break;
    }
  }
  return targetList;
}

List* ExpandAllTables(ParseState* pstate) {
  List* targetList = nullptr;
  ListCell* cell;
  foreach (cell, pstate->rtable) {
    RangeTblEntry* rte = (RangeTblEntry*) lfirst(cell);

    targetList = list_concat(targetList, ExpandSingleTable(pstate, rte));
  }
  return targetList;
}

TargetEntry* TransformTargetEntry(ParseState* pstate, RangeTblEntry* rte, Node* node, char* colname) {
  char* column = strVal((Value*) node);;
  int rtindex = RTERangeTablePosn(pstate, rte, nullptr);

  switch (rte->rteKind) {
    case RTE_RELATION: {
      Relation* rel = Relation::OpenTable(CurrentTransaction, rte->relid);

      for (SysAttribute& attr : rel->rd_attr) {
        if (strcmp(column, attr.attname.data) == 0) {
          Var* var = makeVar(rtindex, attr.attnum, attr.atttypid);
          TargetEntry* te = makeTargetEntry((Expr*) var, pstate->p_next_resno++, colname);
          if (!te->resname) {
            te->resname = column;
          }
          return te;
        }
      }
      elog(ERROR, "column %s does not exist", column);
      break;
    }
    default: {
      elog(ERROR, "invalid rtekind %d", rte->rteKind);
      break;
    }
  }

}

List* TransformColumnRef(ParseState* pstate, ColumnRef* cref, char* columnName) {
  int fields = list_length(cref->fields);
  Node* node1;
  Node* node2;
  switch (fields) {
    case 1: {
      node1 = (Node*) linitial(cref->fields);
      assert(node1->type == T_String);
      char* column = strVal((Value*) node1);
      Var* var = colNameToVar(pstate, column);
      if (!var) {
        elog(ERROR, "column %s does not exist", column);
      }

      TargetEntry* te = makeTargetEntry((Expr*) var, pstate->p_next_resno++, columnName);
      if (!te->resname) {
        te->resname = column;
      }

      return list_make1(te);
    }

    case 2: {
      node1 = (Node*) linitial(cref->fields);
      node2 = (Node*) llast(cref->fields);
      assert(node1->type == T_String);
      char* relname = strVal((Value*) node1);
      RangeTblEntry* rte = refnameRangeTblEntry(pstate, relname, nullptr);
      if (!rte) {
        elog(ERROR, "missing FROM-clause entry for table '%s'", relname);
      }

      if (node2->type == T_A_Star) {
        /*
         * Target item is relation.*, expand that table
         *
         * (e.g., SELECT emp.*, dname FROM emp, dept)
         */
        return ExpandSingleTable(pstate, rte);

      } else if (node2->type == T_String) {
        /*
         * Not "something.*", or we want to treat that as a plain whole-row
         * variable, so transform as a single expression
         */

        return list_make1(TransformTargetEntry(pstate, rte, node2, columnName));
      }
      break;
    }
  }
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
      p_target = list_concat(p_target, ExpandAllTables(pstate));
      continue;
    }

    if (res->val->type == T_ColumnRef) {
      ColumnRef* cref = (ColumnRef*) res->val;
      p_target = list_concat(p_target, TransformColumnRef(pstate, cref, res->name));
      continue;
    }

    elog(ERROR, "invalid target type %d", res->val->type);
  }

  return p_target;
}

}
