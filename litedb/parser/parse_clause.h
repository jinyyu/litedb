#ifndef LITEDB_PARSER_PARSE_CLAUSE_H_
#define LITEDB_PARSER_PARSE_CLAUSE_H_
#include <litedb/nodes/parsenodes.h>

namespace db {

void TransformFromClause(ParseState* pstate, List* fromClause);
Node* TransformWhereClause(ParseState* pstate, Node* clause);

}

#endif //LITEDB_PARSER_PARSE_CLAUSE_H_
