#ifndef LITESQL_SRC_PARSER_NODES_H_
#define LITESQL_SRC_PARSER_NODES_H_
#include <litesql/mcxt.h>
#include <list>

namespace db {
enum NodeTag {
  T_Invalid = 0,
  T_SchemaOptName
};

#define newNode(size, tag) \
({    Node   *_result; \
    _result = (Node *) Malloc0(size); \
    _result->type = (tag); \
    _result; \
})
#define makeNode(_type_) ((_type_ *) newNode(sizeof(_type_),T_##_type_))

struct Node {
  NodeTag type;
};

struct NodeList : public Object {
  explicit NodeList() : Object(CurTransactionContext) {
  }

  explicit NodeList(Node* node) : Object(CurTransactionContext) {
    nodes.push_back(node);
  }

  explicit NodeList(Node* node1, Node* node2) : Object(CurTransactionContext) {
    nodes.push_back(node1);
    nodes.push_back(node2);
  }

  void Append(Node* node) {
    nodes.push_back(node);
  }
  std::list<Node*> nodes;
};

};
#endif //LITESQL_SRC_PARSER_NODES_H_
