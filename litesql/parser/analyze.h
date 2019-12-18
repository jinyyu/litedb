#ifndef LITESQL_LITESQL_PARSER_ANALYZE_H_
#define LITESQL_LITESQL_PARSER_ANALYZE_H_
#include <litesql/parser/nodes.h>

namespace db {

typedef enum CmdType {
  CMD_UNKNOWN,
  CMD_CREATE,      /*  create stmt */
  CMD_SELECT,      /* select stmt */
  CMD_UPDATE,      /* update stmt */
  CMD_INSERT,      /* insert stmt */
  CMD_DELETE,      /* delete stmt */
} CmdType;

struct Query {
  NodeTag type;
  CmdType commandType;    /* create|select|insert|update|delete */
  Node* utilityStmt;       /* non-null if commandType == CMD_CREATE */
};

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

#endif //LITESQL_LITESQL_PARSER_ANALYZE_H_
