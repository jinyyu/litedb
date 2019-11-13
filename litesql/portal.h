#ifndef LITESQL_LITESQL_PORTAL_H_
#define LITESQL_LITESQL_PORTAL_H_
#include <litesql/mcxt.h>
#include <litesql/nodes.h>

namespace db {

struct Portal : Object {
  explicit Portal(Node* parseTree, MemoryContext* portalContext)
      : Object(portalContext),
        portalContext(portalContext),
        parseTree(parseTree) {
  }

  virtual ~Portal() {

  }

  void Run();

  MemoryContext* portalContext;
  Node* parseTree;
};

}
#endif //LITESQL_LITESQL_PORTAL_H_
