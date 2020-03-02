#include <litedb/parser/parse_relation.h>
#include <litedb/utils/elog.h>
#include <litedb/catalog/catalog.h>
#include <litedb/storage/relation.h>

namespace db {

/*
 * given an RTE, return RT index (starting with 1) of the entry,
 * and optionally get its nesting depth (0 = current).  If sublevels_up
 * is NULL, only consider rels at the current nesting level.
 * Raises error if RTE not found.
 */
int RTERangeTablePosn(ParseState* pstate, RangeTblEntry* rte, int* sublevels_up) {
  int index;

  if (sublevels_up)
    *sublevels_up = 0;

  while (pstate != nullptr) {
    index = 1;
    ListCell* cell;
    foreach (cell, pstate->rtable) {
      RangeTblEntry* r = (RangeTblEntry*) lfirst(cell);
      if (rte == r)
        return index;
      index++;
    }
    pstate = pstate->parentParseState;
    if (sublevels_up)
      (*sublevels_up)++;
    else
      break;
  }

  elog(ERROR, "RTE not found (internal error)");
  return 0;                    /* keep compiler quiet */
}

/*
 * refnameRangeTblEntry
 *	  Given a possibly-qualified refname, look to see if it matches any RTE.
 *	  If so, return a pointer to the RangeTblEntry; else return NULL.
 */
RangeTblEntry* refnameRangeTblEntry(ParseState* pstate,
                                    const char* refname,
                                    int* sublevels_up) {
  while (pstate != NULL) {
    RangeTblEntry* result = NULL;
    ListCell* cell;
    foreach(cell, pstate->rtable) {
      RangeTblEntry* rte = (RangeTblEntry*) lfirst(cell);

      if (rte->alias && strcmp(refname, rte->alias) == 0) {
        result = rte;
        break;
      }

      if (rte->rteKind == RTE_RELATION && rte->relid) {
        Relation* rel = Relation::OpenTable(CurrentTransaction, rte->relid);
        if (strcmp(refname, NameStr(&rel->rd_rel.relname)) == 0) {
          result = rte;
          break;
        }
      }
    }
    if (result)
      return result;

    if (sublevels_up)
      (*sublevels_up)++;
    else
      break;

    pstate = pstate->parentParseState;
  }
  return NULL;
}

/*
 * colNameToVar
 *	  Search for an unqualified column name.
 *	  If found, return the appropriate Var node (or expression).
 */
Node* colNameToVar(ParseState* pstate, const char* colname) {
  Node* result = NULL;
  while (pstate) {
    ListCell* cell;
    foreach(cell, pstate->rtable) {
      RangeTblEntry* rte = (RangeTblEntry*) lfirst(cell);
      Node* netResult = scanRTEForColumn(pstate, rte, colname);

      if (netResult) {
        if (result) {
          elog(ERROR, "column reference \"%s\" is ambiguous", colname);
        }
        result = netResult;
      }
    }
    pstate = pstate->parentParseState;
  }
  return result;
}

/*
 * scanRTEForColumn
 *	  Search the column names of a single RTE for the given name.
 *	  If found, return an appropriate Var node, else return NULL.
 */
Node* scanRTEForColumn(ParseState* pstate, RangeTblEntry* rte, const char* colname) {
  Node* result = NULL;
  if (rte->rteKind == RTE_RELATION && rte->relid) {
    Relation* rel = Relation::OpenTable(CurrentTransaction, rte->relid);
    for (SysAttribute& attr : rel->rd_attr) {

      if (strcmp(colname, attr.attname.data) == 0) {

        if (result) {
          elog(ERROR, "column reference \"%s\" is ambiguous", colname);
        }
        int rtindex = RTERangeTablePosn(pstate, rte, nullptr);
        result = (Node*) makeVar(rtindex, attr.attnum, attr.atttypid);
      }
    }
  }

  return result;
}

}

