#ifndef LITEDB_PORTAL_H_
#define LITEDB_PORTAL_H_
#include <litedb/nodes/parsenodes.h>

namespace db {

class Session;

struct Portal {
  explicit Portal(Session* session, List* planTrees)
      : session(session),
        planTrees(planTrees) {
  }

  ~Portal() {

  }

  void Start() {}

  void Run();

  /*
   * utility to get a string representation of the command operation,
   */
  static const char* CreateCommandTag(Node* parseTree);

  Session* session;
  List* planTrees;
  const char* commandTag;
};

}
#endif //LITEDB_PORTAL_H_
