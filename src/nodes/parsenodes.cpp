#include <litedb/nodes/parsenodes.h>
#include <litedb/utils/elog.h>
#include <assert.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <functional>
#include <litedb/nodes/value.h>

namespace db {

typedef rapidjson::Value (* NodeDumpFunc)(Node* node, rapidjson::Document& doc);
static rapidjson::Value dumpNode(Node* node, rapidjson::Document& doc);
static rapidjson::Value dumpArray(List<Node>* nodeList, rapidjson::Document& doc);
static rapidjson::Value notImpl(CreateTableStmt* node, rapidjson::Document& doc);
static rapidjson::Value dumpCreateTableStmt(CreateTableStmt* node, rapidjson::Document& doc);
static rapidjson::Value dumpTypename(Typename* node, rapidjson::Document& doc);
static rapidjson::Value dumpColumnDef(ColumnDef* node, rapidjson::Document& doc);
static rapidjson::Value dumpValue(Value* node, rapidjson::Document& doc);
static rapidjson::Value dumpColumnConstraint(ColumnConstraint* node, rapidjson::Document& doc);
static rapidjson::Value dumpTableConstraint(TableConstraint* node, rapidjson::Document& doc);

static const char* ConstraintTypeStr(ConstraintType type);
static const char* ConflictAlgorithmStr(ConflictAlgorithm type);
static NodeDumpFunc GetNodeDumpFunction(NodeTag tag, const char** tagName);

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
      name = "T_Value";
      break;
    }
    case T_TableConstraint: {
      func = (NodeDumpFunc) dumpTableConstraint;
      name = "T_TableConstraint";
      break;
    }
    default: {
      elog(INFO, "not impl tag type %d", type);
      func = (NodeDumpFunc) notImpl;
      name = "Invalid";
      break;
    }
  }
  if (tagName) {
    *tagName = name;
  }
  return func;
}

rapidjson::Value notImpl(CreateTableStmt* node, rapidjson::Document& doc) {
  rapidjson::Value value(rapidjson::kObjectType);
  const char* msg = "no implement";
  int len = strlen(msg);
  value.SetString(msg, len);
  return value;
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
  std::string buff;
  buff.append(node->name);
  if (node->leftPrecision) {
    char tmp[16];
    snprintf(tmp, sizeof(tmp), "(%d", node->leftPrecision);
    buff.append(tmp);
  }
  if (node->rightPrecision) {
    char tmp[16];
    snprintf(tmp, sizeof(tmp), ",%d", node->rightPrecision);
    buff.append(tmp);
  }
  if (node->leftPrecision || node->rightPrecision) {
    buff.push_back(')');
  }
  rapidjson::Value value;
  value.SetString(buff.c_str(), buff.size(), doc.GetAllocator());
  return value;
}

rapidjson::Value dumpColumnDef(ColumnDef* node, rapidjson::Document& doc) {
  rapidjson::Value value(rapidjson::kObjectType);
  value.AddMember("columnName", rapidjson::Value(node->columnName, doc.GetAllocator()), doc.GetAllocator());
  value.AddMember("typeName", dumpNode((Node*) node->typeName, doc), doc.GetAllocator());
  if (node->constraintName) {
    value.AddMember("constraintName", dumpNode((Node*) node->constraintName, doc), doc.GetAllocator());
  }
  if (node->columnConstraints) {
    value.AddMember("columnConstraints", dumpArray(node->columnConstraints, doc), doc.GetAllocator());
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

  if (node->conflictAlgorithm != CONFLICT_DEFAULT) {
    value.AddMember("conflictAlgorithm",
                    rapidjson::Value(ConflictAlgorithmStr(node->conflictAlgorithm), doc.GetAllocator()),
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
  if (node->conflictAlgorithm != CONFLICT_DEFAULT) {
    value.AddMember("conflictAlgorithm",
                    rapidjson::Value(ConflictAlgorithmStr(node->conflictAlgorithm), doc.GetAllocator()),
                    doc.GetAllocator());
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

const char* ConflictAlgorithmStr(ConflictAlgorithm type) {
  switch (type) {
    case CONFLICT_ROLLBACK: {
      return "ROLLBACK";
    }
    case CONFLICT_ABORT: {
      return "ABORT";
    }
    case CONFLICT_FAIL: {
      return "FAIL";
    }
    case CONFLICT_IGNORE: {
      return "IGNORE";
    }
    case CONFLICT_REPLACE: {
      return "REPLACE";
    }
    default: {
      elog(ERROR, "invalid conflict algorithm %d", type);
    }
  }
}

rapidjson::Value dumpNode(Node* node, rapidjson::Document& doc) {
  NodeDumpFunc nodeDumpFunc = GetNodeDumpFunction(node->type, nullptr);
  return nodeDumpFunc(node, doc);
}

rapidjson::Value dumpArray(List<Node>* nodeList, rapidjson::Document& doc) {
  rapidjson::Value array(rapidjson::kArrayType);
  for (Node* node: nodeList->list) {
    auto value = dumpNode(node, doc);
    array.PushBack(value, doc.GetAllocator());
  }
  return array;
}

void NodeDisplay(Node* node) {
  assert(node);

  rapidjson::Document doc(rapidjson::kObjectType);

  const char* name;
  NodeDumpFunc nodeDumpFunc = GetNodeDumpFunction(node->type, &name);

  rapidjson::Value value = nodeDumpFunc(node, doc);
  doc.AddMember(rapidjson::Value(name, doc.GetAllocator()), value, doc.GetAllocator());

  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  writer.SetIndent(' ', 2);
  doc.Accept(writer);
  const char* out = buffer.GetString();
  fprintf(stdout, "%s\n", out);
}

}