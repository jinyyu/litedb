#include <litedb/utils/portal.h>
#include <litedb/nodes/parsenodes.h>
#include <litedb/utils/elog.h>
#include <litedb/bin/session.h>

namespace db {

const char* Portal::CreateCommandTag(Node* parseTree) {
  const char* tag;
  switch (parseTree->type) {
    case T_CreateTableStmt: {
      tag = "CREATE TABLE";
      break;
    }
    default: {
      tag = "???";
      break;
    }
  }
  return tag;
}

void Portal::Run() {

}

}