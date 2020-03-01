#ifndef LITEDB_NODES_NODES_H_
#define LITEDB_NODES_NODES_H_

namespace db
{

enum NodeTag {
  T_Invalid = 0,

  /*
 * TAGS FOR VALUE NODES (value.h)
 */
  T_Value,
  T_Integer,
  T_Float,
  T_String,
  T_Null,

  T_List,
  T_IntList,

  T_A_Expr,
  T_A_Const,
  T_ColumnRef,

  T_CreateTableStmt,
  T_Typename,
  T_ColumnDef,
  T_ColumnConstraint,
  T_TableConstraint,
  T_SelectStmt,
  T_ResTarget,
  T_RangeVar,
  T_A_Star,

  T_Query,
  T_RangeTblEntry,
  T_RangeTblRef,

  T_NullTest,
  T_BooleanTest,
  T_BoolExpr,
  T_Var,
  T_TargetEntry,
  T_FromExpr,

  T_PlannedStmt,
};

#define newNode(size, tag) \
({    Node   *_result; \
    _result = (Node *) SessionEnv->Malloc0(size); \
    _result->type = (tag); \
    _result; \
})
#define makeNode(_type_) ((_type_ *) newNode(sizeof(_type_),T_##_type_))

struct Node {
  NodeTag type;
};


}

#endif //LITEDB_NODES_NODES_H_
