#ifndef LITESQL_SRC_PARSER_NODES_H_
#define LITESQL_SRC_PARSER_NODES_H_
#include <litesql/utils/memctx.h>
#include <list>

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

struct NodeList : public Object {
  explicit NodeList() : Object(CurTransactionContext) {
  }

  explicit NodeList(Node* node) : Object(CurTransactionContext) {
    nodes.push_back(node);
  }

  explicit NodeList(Node* node1, Node* node2) : Object(CurTransactionContext) {
    nodes.push_back(node1);
    nodes.push_back(node2);
  }

  void Append(Node* node) {
    nodes.push_back(node);
  }
  std::list<Node*> nodes;
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
  NodeList* columnConstraints;
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
  NodeList* columnList;
  ConflictAlgorithm conflictAlgorithm;
  Expr* expr;
};

struct CreateTableStmt {
  NodeTag type;
  bool temp;
  char* name;
  NodeList* columns;
  NodeList* table_constraints;
};

void NodeDisplay(Node* node);

};
#endif //LITESQL_SRC_PARSER_NODES_H_
