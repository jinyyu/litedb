#ifndef LITEDB_NODES_EXECNODES_H_
#define LITEDB_NODES_EXECNODES_H_
#include <litedb/int.h>
#include <litedb/nodes/nodes.h>

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

struct IndexInfo {
  int ii_NumIndexKeyAttrs;    /* number of key columns in index */
  i16 ii_IndexAttrNumbers[INDEX_MAX_KEYS];
  bool ii_Unique;
};

}

#endif //LITEDB_NODES_EXECNODES_H_
