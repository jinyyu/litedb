#include <litedb/nodes/parsenodes.h>
#include <litedb/utils/elog.h>
#include <assert.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <functional>
#include <litedb/nodes/value.h>
#include <litedb/nodes/execnodes.h>

namespace db {

typedef rapidjson::Value (* NodeDumpFunc)(Node* node, rapidjson::Document& doc);
static rapidjson::Value dumpNode(Node* node, rapidjson::Document& doc);
static rapidjson::Value dumpArray(List* nodeList, rapidjson::Document& doc);
static rapidjson::Value dumpCreateTableStmt(CreateTableStmt* node, rapidjson::Document& doc);
static rapidjson::Value dumpTypename(Typename* node, rapidjson::Document& doc);
static rapidjson::Value dumpColumnDef(ColumnDef* node, rapidjson::Document& doc);
static rapidjson::Value dumpValue(Value* node, rapidjson::Document& doc);
static rapidjson::Value dumpColumnConstraint(ColumnConstraint* node, rapidjson::Document& doc);
static rapidjson::Value dumpTableConstraint(TableConstraint* node, rapidjson::Document& doc);
static rapidjson::Value dumpSelectStmt(SelectStmt* stmt, rapidjson::Document& doc);
static rapidjson::Value dumpResTarget(ResTarget* node, rapidjson::Document& doc);
static rapidjson::Value dumpRangeVar(RangeVar* node, rapidjson::Document& doc);
static rapidjson::Value dumpA_Star(RangeVar* node, rapidjson::Document& doc);
static rapidjson::Value dumpColumnRef(ColumnRef* node, rapidjson::Document& doc);
static rapidjson::Value dumpA_Expr(A_Expr* node, rapidjson::Document& doc);
static rapidjson::Value dumpQuery(Query* node, rapidjson::Document& doc);
static rapidjson::Value dumpVar(Var* node, rapidjson::Document& doc);
static rapidjson::Value dumpTargetEntry(TargetEntry* node, rapidjson::Document& doc);
static rapidjson::Value dumpFromExpr(FromExpr* node, rapidjson::Document& doc);
static rapidjson::Value dumpRangeTblEntry(RangeTblEntry* node, rapidjson::Document& doc);
static rapidjson::Value dumpRangeTblRef(RangeTblRef* node, rapidjson::Document& doc);

static const char* ConstraintTypeStr(ConstraintType type);
static NodeDumpFunc GetNodeDumpFunction(NodeTag type, const char** tagName);

static NodeDumpFunc GetNodeDumpFunction(NodeTag type, const char** tagName) {
  NodeDumpFunc func;
  const char* name;
  switch (type) {
    case T_CreateTableStmt: {
      func = (NodeDumpFunc) dumpCreateTableStmt;
      name = "CreateTableStmt";
      break;
    }
    case T_Typename: {
      func = (NodeDumpFunc) dumpTypename;
      name = "Typename";
      break;
    }
    case T_ColumnDef: {
      func = (NodeDumpFunc) dumpColumnDef;
      name = "T_ColumnDef";
      break;
    }
    case T_ColumnConstraint: {
      func = (NodeDumpFunc) dumpColumnConstraint;
      name = "T_ColumnConstraint";
      break;
    }
    case T_Integer:
    case T_String:
    case T_Float:
    case T_Null: {
      func = (NodeDumpFunc) dumpValue;
      name = "Value";
      break;
    }
    case T_List:
    case T_IntList: {
      func = (NodeDumpFunc) dumpArray;
      name = "list";
      break;
    }
    case T_TableConstraint: {
      func = (NodeDumpFunc) dumpTableConstraint;
      name = "TableConstraint";
      break;
    }
    case T_SelectStmt: {
      func = (NodeDumpFunc) dumpSelectStmt;
      name = "SelectStmt";
      break;
    }
    case T_ResTarget: {
      func = (NodeDumpFunc) dumpResTarget;
      name = "ResTarget";
      break;
    }
    case T_RangeVar: {
      func = (NodeDumpFunc) dumpRangeVar;
      name = "RangeVar";
      break;
    }
    case T_A_Star: {
      func = (NodeDumpFunc) dumpA_Star;
      name = "A_Star";
      break;
    }

    case T_ColumnRef: {
      func = (NodeDumpFunc) dumpColumnRef;
      name = "ColumnRef";
      break;
    }
    case T_A_Expr: {
      func = (NodeDumpFunc) dumpA_Expr;
      name = "A_Expr";
      break;
    }
    case T_Query: {
      func = (NodeDumpFunc) dumpQuery;
      name = "Query";
      break;
    }
    case T_Var: {
      func = (NodeDumpFunc) dumpVar;
      name = "Var";
      break;
    }
    case T_TargetEntry: {
      func = (NodeDumpFunc) dumpTargetEntry;
      name = "TargetEntry";
      break;
    }
    case T_FromExpr: {
      func = (NodeDumpFunc) dumpFromExpr;
      name = "FromExpr";
      break;
    }
    case T_RangeTblEntry: {
      func = (NodeDumpFunc) dumpRangeTblEntry;
      name = "RangeTblEntry";
      break;
    }
    case T_RangeTblRef: {
      func = (NodeDumpFunc) dumpRangeTblRef;
      name = "RangeTblRef";
      break;
    }
    default: {
      elog(ERROR, "not impl tag type %d", type);
      name = "Invalid";
      func = nullptr;
      break;
    }
  }
  if (tagName) {
    *tagName = name;
  }
  return func;
}

rapidjson::Value dumpCreateTableStmt(CreateTableStmt* node, rapidjson::Document& doc) {
  rapidjson::Value value(rapidjson::kObjectType);

  value.AddMember("temp", node->temp, doc.GetAllocator());
  value.AddMember("name", rapidjson::Value(node->name, doc.GetAllocator()), doc.GetAllocator());

  if (node->columns) {
    rapidjson::Value array = dumpArray(node->columns, doc);
    value.AddMember("columns", array, doc.GetAllocator());
  }

  if (node->table_constraints) {
    rapidjson::Value array = dumpArray(node->table_constraints, doc);
    value.AddMember("table_constraints", array, doc.GetAllocator());
  }
  return value;
}

rapidjson::Value dumpTypename(Typename* node, rapidjson::Document& doc) {
  char buffer[128];
  size_t n;
  if (node->typeMod > 0) {
    n = snprintf(buffer, sizeof(buffer), "%s(%d)", node->name, node->typeMod);
  } else {
    n = snprintf(buffer, sizeof(buffer), "%s", node->name);
  }

  rapidjson::Value value;
  value.SetString(buffer, n, doc.GetAllocator());
  return value;
}

rapidjson::Value dumpColumnDef(ColumnDef* node, rapidjson::Document& doc) {
  rapidjson::Value value(rapidjson::kObjectType);
  value.AddMember("columnName", rapidjson::Value(node->columnName, doc.GetAllocator()), doc.GetAllocator());
  value.AddMember("typeName", dumpNode((Node*) node->typeName, doc), doc.GetAllocator());
  if (node->constraintName) {
    value.AddMember("constraintName", dumpNode((Node*) node->constraintName, doc), doc.GetAllocator());
  }
  if (node->constraints) {
    value.AddMember("constraints", dumpArray(node->constraints, doc), doc.GetAllocator());
  }
  return value;
}

rapidjson::Value dumpValue(Value* node, rapidjson::Document& doc) {
  rapidjson::Value value;
  switch (node->type) {
    case T_Integer: {
      value.SetInt(intVal(node));
      break;
    }
    case T_String: {
      char* str = strVal(node);
      value.SetString(str, strlen(str));
      break;
    }
    case T_Float: {
      value.SetFloat(floatVal(node));
      break;
    }
    case T_Null: {
      value.SetString("NULL");
    }
    default: {
      elog(ERROR, "invalid value type %d", node->type);
      break;
    }
  }
  return value;
}

rapidjson::Value dumpColumnConstraint(ColumnConstraint* node, rapidjson::Document& doc) {
  rapidjson::Value value(rapidjson::kObjectType);
  if (node->constraint != CONSTRAINT_NONE) {
    value.AddMember("constraint",
                    rapidjson::Value(ConstraintTypeStr(node->constraint), doc.GetAllocator()),
                    doc.GetAllocator());
  }

  if (node->defaultValue) {
    value.AddMember("defaultValue", dumpNode((Node*) node->defaultValue, doc), doc.GetAllocator());
  }
  return value;
}

rapidjson::Value dumpTableConstraint(TableConstraint* node, rapidjson::Document& doc) {
  rapidjson::Value value(rapidjson::kObjectType);
  if (node->constraint != CONSTRAINT_NONE) {
    value.AddMember("constraint",
                    rapidjson::Value(ConstraintTypeStr(node->constraint), doc.GetAllocator()),
                    doc.GetAllocator());
  }
  if (node->columnList) {
    value.AddMember("columnList", dumpArray(node->columnList, doc), doc.GetAllocator());
  }
  return value;
}

const char* ConstraintTypeStr(ConstraintType type) {
  switch (type) {
    case CONSTRAINT_NOT_NULL: {
      return "NOT NULL";
    }
    case CONSTRAINT_PRIMARY_KEY: {
      return "PRIMARY KEY";
    }
    case CONSTRAINT_UNIQUE: {
      return "UNIQUE";
    }
    case CONSTRAINT_CHECK: {
      return "CHECK";
    }
    case CONSTRAINT_DEFAULT: {
      return "DEFAULT";
    }
    default: {
      elog(ERROR, "invalid constraint type %d", type);
    }
  }
}

rapidjson::Value dumpSelectStmt(SelectStmt* stmt, rapidjson::Document& doc) {
  rapidjson::Value value(rapidjson::kObjectType);

  value.AddMember("distinct", stmt->distinct, doc.GetAllocator());

  if (stmt->targetList) {
    value.AddMember("targetList", dumpArray(stmt->targetList, doc), doc.GetAllocator());
  }

  if (stmt->fromClause) {
    value.AddMember("fromClause", dumpArray(stmt->fromClause, doc), doc.GetAllocator());
  }

  if (stmt->whereClause) {
    value.AddMember("whereClause", dumpNode(stmt->whereClause, doc), doc.GetAllocator());
  }
  return value;
}

rapidjson::Value dumpResTarget(ResTarget* node, rapidjson::Document& doc) {
  rapidjson::Value value(rapidjson::kObjectType);
  if (node->name) {
    value.AddMember("name",
                    rapidjson::Value(node->name, doc.GetAllocator()),
                    doc.GetAllocator());
  }

  if (node->val) {
    value.AddMember("val",
                    dumpNode(node->val, doc),
                    doc.GetAllocator());
  }
  return value;
}

rapidjson::Value dumpRangeVar(RangeVar* node, rapidjson::Document& doc) {
  rapidjson::Value value(rapidjson::kObjectType);
  if (node->relname) {
    value.AddMember("relname",
                    rapidjson::Value(node->relname, doc.GetAllocator()),
                    doc.GetAllocator());
  }

  if (node->alias) {
    value.AddMember("alias",
                    rapidjson::Value(node->alias, doc.GetAllocator()),
                    doc.GetAllocator());
  }
  return value;
}

rapidjson::Value dumpA_Star(RangeVar* node, rapidjson::Document& doc) {
  rapidjson::Value value;
  value.SetString("*");
  return value;
}

rapidjson::Value dumpColumnRef(ColumnRef* node, rapidjson::Document& doc) {
  rapidjson::Value array = dumpArray(node->fields, doc);
  rapidjson::Value value(rapidjson::kObjectType);
  value.AddMember("fields", array, doc.GetAllocator());
  return value;
}

rapidjson::Value dumpA_Expr(A_Expr* node, rapidjson::Document& doc) {
  rapidjson::Value value(rapidjson::kObjectType);
  value.AddMember("kind", rapidjson::Value(node->kind), doc.GetAllocator());
  value.AddMember("name", rapidjson::Value(node->name, doc.GetAllocator()), doc.GetAllocator());
  if (node->lexpr) {
    value.AddMember("lexpr", dumpNode(node->lexpr, doc), doc.GetAllocator());
  }
  if (node->rexpr) {
    value.AddMember("rexpr", dumpNode(node->rexpr, doc), doc.GetAllocator());
  }
  return value;
}

rapidjson::Value dumpQuery(Query* node, rapidjson::Document& doc) {
  rapidjson::Value value(rapidjson::kObjectType);
  if (node->utilityStmt) {
    value.AddMember("utilityStmt", dumpNode(node->utilityStmt, doc), doc.GetAllocator());
  }
  if (node->rtable) {
    value.AddMember("rtable", dumpArray(node->rtable, doc), doc.GetAllocator());
  }

  if (node->jointree) {
    value.AddMember("jointree", dumpNode((Node*) node->jointree, doc), doc.GetAllocator());
  }

  if (node->targetList) {
    value.AddMember("targetList", dumpArray(node->targetList, doc), doc.GetAllocator());
  }
  return value;
}

rapidjson::Value dumpVar(Var* node, rapidjson::Document& doc) {
  rapidjson::Value value(rapidjson::kObjectType);
  value.AddMember("varno", rapidjson::Value(node->varno), doc.GetAllocator());
  value.AddMember("varattno", rapidjson::Value(node->varattno), doc.GetAllocator());
  value.AddMember("vartype", rapidjson::Value(node->vartype), doc.GetAllocator());
  return value;
}

rapidjson::Value dumpTargetEntry(TargetEntry* node, rapidjson::Document& doc) {
  rapidjson::Value value(rapidjson::kObjectType);
  if (node->expr) {
    value.AddMember("expr", dumpNode((Node*) node->expr, doc), doc.GetAllocator());
  }
  value.AddMember("resno", rapidjson::Value(node->resno), doc.GetAllocator());

  if (node->resname) {
    value.AddMember("resname", rapidjson::Value(node->resname, doc.GetAllocator()), doc.GetAllocator());
  }
  return value;
}

rapidjson::Value dumpFromExpr(FromExpr* node, rapidjson::Document& doc) {
  rapidjson::Value value(rapidjson::kObjectType);
  if (node->quals) {
    value.AddMember("quals", dumpNode(node->quals, doc), doc.GetAllocator());
  }

  if (node->fromlist) {
    value.AddMember("fromlist", dumpArray(node->fromlist, doc), doc.GetAllocator());
  }
  return value;
}

rapidjson::Value dumpRangeTblEntry(RangeTblEntry* node, rapidjson::Document& doc) {
  rapidjson::Value value(rapidjson::kObjectType);
  value.AddMember("rteKind", rapidjson::Value(node->rteKind), doc.GetAllocator());

  value.AddMember("relid", rapidjson::Value(node->relid), doc.GetAllocator());

  char relkind[2];
  snprintf(relkind, sizeof(relkind), "%c", node->relkind);
  value.AddMember("relkind", rapidjson::Value(relkind, 1, doc.GetAllocator()), doc.GetAllocator());

  if (node->alias) {
    value.AddMember("alias", rapidjson::Value(node->alias, doc.GetAllocator()), doc.GetAllocator());
  }
  return value;
}

rapidjson::Value dumpRangeTblRef(RangeTblRef* node, rapidjson::Document& doc) {
  rapidjson::Value value(rapidjson::kObjectType);
  value.AddMember("rtindex", rapidjson::Value(node->rtindex), doc.GetAllocator());
  return value;
}

rapidjson::Value dumpNode(Node* node, rapidjson::Document& doc) {
  const char* name;
  rapidjson::Value value(rapidjson::kObjectType);

  NodeDumpFunc nodeDumpFunc = GetNodeDumpFunction(node->type, &name);
  value.AddMember(rapidjson::Value(name, strlen(name)), nodeDumpFunc(node, doc), doc.GetAllocator());
  return value;
}

rapidjson::Value dumpArray(List* nodeList, rapidjson::Document& doc) {
  rapidjson::Value array(rapidjson::kArrayType);
  if (nodeList->type == T_List) {
    ListCell* cell;
    foreach (cell, nodeList) {
      Node* node = (Node*) lfirst(cell);
      auto value = dumpNode(node, doc);
      array.PushBack(value, doc.GetAllocator());
    }

  } else if (nodeList->type == T_IntList) {
    ListCell* cell;
    foreach (cell, nodeList) {
      int v = lfirst_int(cell);
      rapidjson::Value value(v);
      array.PushBack(value, doc.GetAllocator());
    }
  } else {
    elog(ERROR, "invalid type %d", nodeList->type);
  }
  return array;
}

void DisplayParseNode(Node* node) {
  assert(node);

  rapidjson::Document doc(rapidjson::kObjectType);

  const char* nodeName;
  NodeDumpFunc nodeDumpFunc = GetNodeDumpFunction(node->type, &nodeName);

  rapidjson::Value value = nodeDumpFunc(node, doc);
  doc.AddMember(rapidjson::Value(nodeName, doc.GetAllocator()), value, doc.GetAllocator());

  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  writer.SetIndent(' ', 2);
  doc.Accept(writer);
  const char* out = buffer.GetString();
  fprintf(stdout, "%s\n", out);
}

A_Expr* makeA_Expr(A_Expr_Kind kind, const char* name, Node* lexpr, Node* rexpr) {
  A_Expr* a = makeNode(A_Expr);
  a->kind = kind;
  a->name = (char*) name;
  a->lexpr = lexpr;
  a->rexpr = rexpr;
  return a;
}

}