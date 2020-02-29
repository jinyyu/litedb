#include <litedb/parser/parse_relation.h>
#include <litedb/utils/elog.h>

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
    for (RangeTblEntry* r: pstate->rtable) {
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
}

