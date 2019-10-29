#ifndef LITESQL_SRC_PARSER_NODES_H_
#define LITESQL_SRC_PARSER_NODES_H_
#include <litesql/mcxt.h>

namespace db {
enum NodeTag {
  T_Invalid = 0,
  T_SchemaOptName
};

#define newNode(size, tag) \
({	Node   *_result; \
	_result = (Node *) Malloc0(size); \
	_result->type = (tag); \
	_result; \
})
#define makeNode(_type_) ((_type_ *) newNode(sizeof(_type_),T_##_type_))

struct Node {
  NodeTag type;
};

struct SchemaOptName {
  NodeTag type;
  char* schema;
  char* name;
};

};
#endif //LITESQL_SRC_PARSER_NODES_H_
