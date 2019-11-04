#include <litesql/nodes.h>
#include <litesql/elog.h>
#include <assert.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <functional>

namespace db {

typedef rapidjson::Value (* NodeDumpFunc)(Node* node, rapidjson::Document& doc);
static rapidjson::Value dumpCreateTableStmt(CreateTableStmt* node, rapidjson::Document& doc);

struct NodeTypeNameDumpFunc {
  NodeTag tag;
  const char* name;
  NodeDumpFunc nodeDumpFunc;
};

static NodeTypeNameDumpFunc nodeTypeNameDumpFunc[] = {
    {T_Invalid, "Invalid", NULL},
    {T_CreateTableStmt, "CreateTableStmt", (NodeDumpFunc) dumpCreateTableStmt},
    {T_Typename, "Typename"},
    {T_ColumnDef, "ColumnDef"},
    {T_ColumnConstraint, "ColumnConstraint"},
    {T_Value, "Value"},
    {T_TableConstraint, "TableConstraint"},
    {T_Name, "Name"},
};

rapidjson::Value dumpCreateTableStmt(CreateTableStmt* node, rapidjson::Document& doc) {
  rapidjson::Value value(rapidjson::kObjectType);

  value.AddMember("temp", node->temp, doc.GetAllocator());
  value.AddMember("name", rapidjson::Value(node->name, doc.GetAllocator()), doc.GetAllocator());
  return value;

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