#ifndef LITEDB_PARSER_PARSE_TARGET_H_
#define LITEDB_PARSER_PARSE_TARGET_H_
#include <litedb/nodes/parsenodes.h>

namespace db {

List* TransformTargetList(ParseState* pstate, List* targetlist);

}

#endif //LITEDB_PARSER_PARSE_TARGET_H_
