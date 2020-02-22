#ifndef LITEDB_PARSER_NODES_H_
#define LITEDB_PARSER_NODES_H_
#include <litedb/utils/env.h>
#include <litedb/utils/list.h>
#include <litedb/nodes/nodes.h>
#include <litedb/int.h>
#include <list>

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
  List<Node>* constraints;
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

struct SelectStmt {
  NodeTag type;
  bool distinct;
  List<Node>* targetList;   /* the target list (of ResTarget) */
  List<Node>* fromClause;   /* the FROM clause */
  Node* whereClause;        /* WHERE qualification */
};

struct ResTarget {
  NodeTag type;
  char* name;            /* column name or NULL */
  Node* val;            /* the value expression to compute or assign */
};

struct RangeVar {
  NodeTag type;
  char* relname;
  char* alias;
};

struct A_Star {
  NodeTag type;
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
  Node* utilityStmt;      /* non-null if commandType == CMD_CMD_UTILITY */
};

typedef enum RTEKind {
  RTE_RELATION,                /* ordinary relation reference */
  RTE_SUBQUERY,                /* subquery in FROM */
  RTE_JOIN,                    /* join */
  RTE_VALUES,                  /* VALUES (<exprlist>), (<exprlist>), ... */
  RTE_RESULT                   /* RTE represents an empty FROM clause; such
                                * RTEs are added by the planner, they're not
                                * present during parsing or rewriting */
} RTEKind;

struct RangeTblEntry {
  NodeTag type;
  RTEKind rteKind;

  i64 relid;      /* id of the relation */
  char relkind;   /*sys_class.relkind*/

  char* alias;    /* user-written alias clause, if any */

};

struct RangeTblRef {
  NodeTag type;
  int rtindex;
};

struct ParseState : public Object {
  explicit ParseState()
      : Object(),
        sourceText(nullptr) {

  }
  virtual ~ParseState() = default;

  const char* sourceText;           /* source text, or NULL if not available */
  std::list<RangeTblEntry*> rtable; /* range table so far */
  std::list<RangeTblRef*> joinlist; /* join items so far */
};

void DisplayParseNode(Node* node);

};
#endif //LITEDB_PARSER_NODES_H_
