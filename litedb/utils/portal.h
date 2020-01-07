#ifndef LITEDB_PORTAL_H_
#define LITEDB_PORTAL_H_
#include <litedb/utils/memctx.h>
#include <litedb/parser/nodes.h>

namespace db {

struct Session;

struct Portal {
  explicit Portal(Session* session, List<Node>* planTrees)
      : session(session),
        planTrees(planTrees) {
  }

  ~Portal() {

    fprintf(stderr, "------------hi\n");

  }

  void Start() {}

  void Run();

  /*
   * utility to get a string representation of the command operation,
   */
  static const char* CreateCommandTag(Node* parseTree);

  Session* session;
  List<Node>* planTrees;
  const char* commandTag;
};

}
#endif //LITEDB_PORTAL_H_
