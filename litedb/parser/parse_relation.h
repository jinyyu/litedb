#ifndef LITEDB_PARSER_PARSE_RELATION_H_
#define LITEDB_PARSER_PARSE_RELATION_H_
#include <litedb/nodes/parsenodes.h>
#include <litedb/nodes/execnodes.h>

namespace db {

int RTERangeTablePosn(ParseState *pstate, RangeTblEntry *rte, int *sublevels_up);

}

#endif //LITEDB_PARSER_PARSE_RELATION_H_
