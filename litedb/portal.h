#ifndef LITEDB_PORTAL_H_
#define LITEDB_PORTAL_H_
#include <litedb/utils/memctx.h>
#include <litedb/parser/nodes.h>

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
#endif //LITEDB_PORTAL_H_
