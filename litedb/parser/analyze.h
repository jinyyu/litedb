#ifndef LITEDB_PARSER_ANALYZE_H_
#define LITEDB_PARSER_ANALYZE_H_
#include <litedb/parser/nodes.h>

namespace db {

typedef enum CmdType {
  CMD_UNKNOWN,
  CMD_SELECT,      /* select stmt */
  CMD_UPDATE,      /* update stmt */
  CMD_INSERT,      /* insert stmt */
  CMD_DELETE,      /* delete stmt */
  CMD_CMD_UTILITY, /* cmds like create etc. */
} CmdType;

struct Query {
  NodeTag type;
  CmdType commandType;    /* select|insert|update|delete|etc */
  Node* utilityStmt;       /* non-null if commandType == CMD_CMD_UTILITY */
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

#endif //LITEDB_PARSER_ANALYZE_H_
