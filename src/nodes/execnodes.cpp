#include <litedb/nodes/execnodes.h>

namespace db {

Var* makeVar(int varno, i16 varattno, i32 vartype) {
  Var* var = makeNode(Var);
  var->varno = varno;
  var->varattno = varattno;
  var->vartype = vartype;
  return var;
}

TargetEntry* makeTargetEntry(Expr* expr, i16 resno, char* resname) {
  TargetEntry* entry = makeNode(TargetEntry);
  entry->expr = expr;
  entry->resno = resno;
  entry->resname = resname;
  return entry;
}

FromExpr* makeFromExpr(List* fromlist, Node* quals) {
  FromExpr* expr = makeNode(FromExpr);
  expr->fromlist = fromlist;
  expr->quals = quals;
  return expr;
}

}

