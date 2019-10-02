#ifndef LITESQL_SRC_PARSER_NODES_H_
#define LITESQL_SRC_PARSER_NODES_H_
namespace db {
enum NodeTag {

};
struct Node {
  NodeTag type;
};

struct RawStmt {
  NodeTag type;
  Node* stmt;            /* raw parse tree */
  int stmtLocation;     /* start location, or -1 if unknown */
  int stmtLen;          /* length in bytes; 0 means "rest of string" */
};

};
#endif //LITESQL_SRC_PARSER_NODES_H_
