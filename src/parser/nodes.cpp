#include <litesql/nodes.h>
#include <litesql/elog.h>
#include <assert.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <functional>

namespace db {

typedef rapidjson::Value (* NodeDumpFunc)(Node* node, rapidjson::Document& doc);
static rapidjson::Value dumpNode(Node* node, rapidjson::Document& doc);
static rapidjson::Value dumpArray(NodeList* node, rapidjson::Document& doc);
static rapidjson::Value notImpl(CreateTableStmt* node, rapidjson::Document& doc);
static rapidjson::Value dumpCreateTableStmt(CreateTableStmt* node, rapidjson::Document& doc);
static rapidjson::Value dumpTypename(Typename* node, rapidjson::Document& doc);
static rapidjson::Value dumpColumnDef(ColumnDef* node, rapidjson::Document& doc);

struct NodeTypeNameDumpFunc {
  NodeTag tag;
  const char* name;
  NodeDumpFunc nodeDumpFunc;
};

static NodeTypeNameDumpFunc nodeTypeNameDumpFunc[] = {
    {T_Invalid, "Invalid", (NodeDumpFunc) notImpl},
    {T_CreateTableStmt, "CreateTableStmt", (NodeDumpFunc) dumpCreateTableStmt},
    {T_Typename, "Typename", (NodeDumpFunc) dumpTypename},
    {T_ColumnDef, "ColumnDef", (NodeDumpFunc) dumpColumnDef},
    {T_ColumnConstraint, "ColumnConstraint", (NodeDumpFunc) notImpl},
    {T_Value, "Value", (NodeDumpFunc) notImpl},
    {T_TableConstraint, "TableConstraint", (NodeDumpFunc) notImpl},
    {T_Name, "Name", (NodeDumpFunc) notImpl},
};

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
  value.AddMember("typeName", dumpNode((Node*)node->typeName, doc), doc.GetAllocator());
  if (node->constraintName) {
    value.AddMember("constraintName", dumpNode((Node*)node->constraintName, doc), doc.GetAllocator());
  }
  if (node->columnConstraints) {
    value.AddMember("columnConstraints", dumpArray(node->columnConstraints, doc), doc.GetAllocator());
  }

  return value;
}

rapidjson::Value dumpNode(Node* node, rapidjson::Document& doc) {
  NodeDumpFunc nodeDumpFunc = nodeTypeNameDumpFunc[node->type].nodeDumpFunc;
  return nodeDumpFunc(node, doc);
}

rapidjson::Value dumpArray(NodeList* node, rapidjson::Document& doc)
{
  rapidjson::Value array(rapidjson::kArrayType);
  for (auto it = node->nodes.begin(); it != node->nodes.end(); ++it) {
    auto value = dumpNode(*it, doc);
    array.PushBack(value, doc.GetAllocator());
  }
  return array;
}

void NodeDisplay(Node* node) {
  assert(node);

  rapidjson::Document doc(rapidjson::kObjectType);

  NodeTag tag = nodeTypeNameDumpFunc[node->type].tag;
  const char* name = nodeTypeNameDumpFunc[node->type].name;
  NodeDumpFunc nodeDumpFunc = nodeTypeNameDumpFunc[node->type].nodeDumpFunc;

  assert(tag == node->type);
  assert(nodeDumpFunc);
  rapidjson::Value value = nodeDumpFunc(node, doc);
  doc.AddMember(rapidjson::Value(name, doc.GetAllocator()), value, doc.GetAllocator());

  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);
  const char* out = buffer.GetString();
  fprintf(stdout, "%s\n", out);
}

}