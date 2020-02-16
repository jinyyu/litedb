#ifndef LITEDB_PARSER_NODES_H_
#define LITEDB_PARSER_NODES_H_
#include <litedb/utils/env.h>
#include <litedb/utils/list.h>
#include <litedb/nodes/nodes.h>

namespace db {

struct Typename {
  NodeTag type;
  char* name;
  int leftPrecision;
  int rightPrecision;
};

struct ColumnDef {
  NodeTag type;
  char* columnName;
  Typename* typeName;
  char* constraintName;
  List<Node>* columnConstraints;
};

enum ConstraintType {
  CONSTRAINT_NONE,
  CONSTRAINT_NOT_NULL,
  CONSTRAINT_PRIMARY_KEY,
  CONSTRAINT_UNIQUE,
  CONSTRAINT_CHECK,
  CONSTRAINT_DEFAULT,
};

struct Value;
struct ColumnConstraint {
  NodeTag type;
  ConstraintType constraint;
  Value* defaultValue;
};

struct Expr {
  NodeTag type;
};

struct TableConstraint {
  NodeTag type;
  ConstraintType constraint;
  List<Node>* columnList;
  Expr* expr;
};

struct CreateTableStmt {
  NodeTag type;
  bool temp;
  char* name;
  List<Node>* columns;
  List<Node>* table_constraints;
};

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

};
#endif //LITEDB_PARSER_NODES_H_
