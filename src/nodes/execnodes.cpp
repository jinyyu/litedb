#include <litedb/nodes/execnodes.h>

namespace db {

Var* makeVar(i16 varattno, i32 vartype) {
  Var* var = makeNode(Var);
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




}

