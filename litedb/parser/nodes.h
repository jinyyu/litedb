#ifndef LITEDB_PARSER_NODES_H_
#define LITEDB_PARSER_NODES_H_
#include <litedb/utils/memctx.h>
#include <litedb/utils/list.h>

namespace db {
enum NodeTag {
  T_Invalid = 0,
  T_CreateTableStmt,
  T_Typename,
  T_ColumnDef,
  T_ColumnConstraint,
  T_Value,
  T_TableConstraint,
  T_Name,
  T_Query,
  T_PlannedStmt,
};

#define newNode(size, tag) \
({    Node   *_result; \
    _result = (Node *) Malloc0(size); \
    _result->type = (tag); \
    _result; \
})
#define makeNode(_type_) ((_type_ *) newNode(sizeof(_type_),T_##_type_))

struct Node {
  NodeTag type;
};

struct Name {
  NodeTag type;
  char* name;
};

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

enum ConflictAlgorithm {
  CONFLICT_DEFAULT,
  CONFLICT_ROLLBACK,
  CONFLICT_ABORT,
  CONFLICT_FAIL,
  CONFLICT_IGNORE,
  CONFLICT_REPLACE,
};

struct Value {
  NodeTag type;
  bool isInt;
  int vInt;
  bool isStr;
  char* str;
  bool isNUll;
};

struct ColumnConstraint {
  NodeTag type;
  ConstraintType constraint;
  ConflictAlgorithm conflictAlgorithm;
  Value* defaultValue;
};

struct Expr {
  NodeTag type;
};

struct TableConstraint {
  NodeTag type;
  ConstraintType constraint;
  List<Node>* columnList;
  ConflictAlgorithm conflictAlgorithm;
  Expr* expr;
};

struct CreateTableStmt {
  NodeTag type;
  bool temp;
  char* name;
  List<Node>* columns;
  List<Node>* table_constraints;
};

void NodeDisplay(Node* node);

};
#endif //LITEDB_PARSER_NODES_H_
