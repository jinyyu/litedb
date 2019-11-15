#ifndef LITESQL_LITESQL_PORTAL_H_
#define LITESQL_LITESQL_PORTAL_H_
#include <litesql/mcxt.h>
#include <litesql/nodes.h>

namespace db {

struct Session;

struct Portal : Object {
  explicit Portal(Session* session, Node* parseTree, MemoryContext* portalContext)
      : Object(portalContext),
        session(session),
        portalContext(portalContext),
        parseTree(parseTree) {
    commandTag = CreateCommandTag(parseTree);
  }

  virtual ~Portal() {

  }

  void Run();

  /*
   * utility to get a string representation of the command operation,
   */
  static const char* CreateCommandTag(Node* parseTree);

  void EndCommand();

  Session* session;
  MemoryContext* portalContext;
  Node* parseTree;
  const char* commandTag;
};

}
#endif //LITESQL_LITESQL_PORTAL_H_
