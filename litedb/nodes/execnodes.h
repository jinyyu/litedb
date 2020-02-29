#ifndef LITEDB_NODES_EXECNODES_H_
#define LITEDB_NODES_EXECNODES_H_
#include <litedb/int.h>
#include <litedb/nodes/nodes.h>
#include <litedb/utils/list.h>

namespace db {

struct Expr {
  NodeTag type;
};

enum NullTestType {
  IS_NULL,
  IS_NOT_NULL
};

struct NullTest {
  Expr xpr;
  Expr* arg;                    /* input expression */
  NullTestType nulltesttype;    /* IS NULL, IS NOT NULL */
  bool argisrow;                /* T to perform field-by-field null checks */
};

enum BoolTestType {
  IS_TRUE,
  IS_NOT_TRUE,
  IS_FALSE,
  IS_NOT_FALSE,
};

struct BooleanTest {
  Expr xpr;
  Expr* arg;                    /* input expression */
  BoolTestType booltesttype;    /* test type */
};

enum BoolExprType {
  AND_EXPR,
  OR_EXPR,
  NOT_EXPR
};

struct BoolExpr {
  Expr xpr;
  BoolExprType boolop;
  List* args;            /* arguments to this expression */
};

struct Var {
  Expr xpr;
  i16 varattno;       /* attribute number of this var, or zero for all attrs ("whole-row Var") */
  i32 vartype;        /* sys_type id for the type of this var */
};

Var* makeVar(i16 varattno, i32 vartype);

struct TargetEntry {
  Expr xpr;
  Expr* expr;          /* expression to evaluate */
  i16 resno;           /* attribute number */
  char* resname;       /* name of the column (could be NULL) */
};

TargetEntry* makeTargetEntry(Expr* expr, i16 resno, char* resname);

struct IndexInfo {
  int ii_NumIndexKeyAttrs;    /* number of key columns in index */
  i16 ii_IndexAttrNumbers[INDEX_MAX_KEYS];
  bool ii_Unique;
};

}

#endif //LITEDB_NODES_EXECNODES_H_
