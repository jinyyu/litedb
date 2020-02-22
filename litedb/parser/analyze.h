#ifndef LITEDB_PARSER_ANALYZE_H_
#define LITEDB_PARSER_ANALYZE_H_
#include <litedb/nodes/parsenodes.h>

namespace db {

/*
 * parse_analyze
 *		Analyze a raw parse tree and transform it to Query form.
 */
Query* ParseAnalyze(Node* parseTree, const char* queryString);

Query* TransformStmt(ParseState* pstate, Node* parseTree);
Query* TransformCreateTableStmt(ParseState* pstate, CreateTableStmt* stmt);
Query* TransformSelectStmt(ParseState *pstate, SelectStmt *stmt);
}

#endif //LITEDB_PARSER_ANALYZE_H_
