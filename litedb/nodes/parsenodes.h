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
  i32 typeMod;
};

struct ColumnDef {
  NodeTag type;
  char* columnName;
  Typename* typeName;
  char* constraintName;
  List* constraints;
};

enum ConstraintType {
  CONSTRAINT_NONE,
  CONSTRAINT_NOT_NULL,
  CONSTRAINT_PRIMARY_KEY,
  CONSTRAINT_UNIQUE,
  CONSTRAINT_CHECK,
  CONSTRAINT_DEFAULT,
};

struct A_Expr;
struct ColumnConstraint {
  NodeTag type;
  ConstraintType constraint;
  A_Expr* defaultValue;
};

enum A_Expr_Kind {
  AEXPR_OP,             /* normal operator */
  AEXPR_OP_ANY,         /* scalar op ANY (array) */
  AEXPR_OP_ALL,         /* scalar op ALL (array) */
  AEXPR_DISTINCT,       /* IS DISTINCT FROM - name must be "=" */
  AEXPR_NOT_DISTINCT,   /* IS NOT DISTINCT FROM - name must be "=" */
  AEXPR_NULLIF,         /* NULLIF - name must be "=" */
  AEXPR_IN,             /* [NOT] IN - name must be "=" or "<>" */
  AEXPR_BETWEEN,        /* name must be "BETWEEN" */
  AEXPR_NOT_BETWEEN,    /* name must be "NOT BETWEEN" */
};

struct A_Expr {
  NodeTag type;
  A_Expr_Kind kind;     /* see above */
  char* name;           /* possibly-qualified name of operator */
  Node* lexpr;          /* left argument, or NULL if none */
  Node* rexpr;          /* right argument, or NULL if none */
};

A_Expr* makeA_Expr(A_Expr_Kind kind, const char* name, Node* lexpr, Node* rexpr);

struct Value;
struct A_Const {
  NodeTag type;
  Value* val;
};

struct ColumnRef {
  NodeTag type;
  List* fields;    /* field names (Value strings) or A_Star */
};

struct TableConstraint {
  NodeTag type;
  ConstraintType constraint;
  List* columnList;
  A_Expr* expr;
};

struct CreateTableStmt {
  NodeTag type;
  bool temp;
  char* name;
  List* columns;
  List* table_constraints;
};

struct SelectStmt {
  NodeTag type;
  bool distinct;
  List* targetList;     /* the target list (of ResTarget) */
  List* fromClause;     /* the FROM clause */
  Node* whereClause;    /* WHERE qualification */
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

  List* targetList;        /* target list (of TargetEntry) */
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
  char relkind;   /* sys_class.relkind */

  char* alias;    /* user-written alias clause, if any */

};

struct RangeTblRef {
  NodeTag type;
  int rtindex;
};

struct ParseState : public Object {
  explicit ParseState()
      : Object(),
        parentParseState(nullptr),
        sourceText(nullptr),
        p_next_resno(1) {

  }
  virtual ~ParseState() = default;

  struct ParseState* parentParseState;    /* stack link */
  const char* sourceText;           /* source text, or NULL if not available */
  std::list<RangeTblEntry*> rtable; /* range table so far */
  std::list<RangeTblRef*> joinlist; /* join items so far */

  i16 p_next_resno;
};

void DisplayParseNode(Node* node, const char* name);

};
#endif //LITEDB_PARSER_NODES_H_
