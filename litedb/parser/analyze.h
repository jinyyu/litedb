#ifndef LITEDB_PARSER_ANALYZE_H_
#define LITEDB_PARSER_ANALYZE_H_
#include <litedb/nodes/parsenodes.h>

namespace db {

struct ParseState {
  const char* sourceText;      /* source text, or NULL if not available */

  static ParseState* Create();
};

/*
 * parse_analyze
 *		Analyze a raw parse tree and transform it to Query form.
 */
Query* ParseAnalyze(Node* parseTree, const char* queryString);

}

#endif //LITEDB_PARSER_ANALYZE_H_
