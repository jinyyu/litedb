#ifndef LITEDB_PARSER_PARSE_RELATION_H_
#define LITEDB_PARSER_PARSE_RELATION_H_
#include <litedb/nodes/parsenodes.h>
#include <litedb/nodes/execnodes.h>

namespace db {

int RTERangeTablePosn(ParseState* pstate, RangeTblEntry* rte, int* sublevels_up);
RangeTblEntry* refnameRangeTblEntry(ParseState* pstate,
                                    const char* refname,
                                    int* sublevels_up);

Var* colNameToVar(ParseState* pstate, const char* colname);

}

#endif //LITEDB_PARSER_PARSE_RELATION_H_
