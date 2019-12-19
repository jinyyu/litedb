#include <litedb/portal.h>
#include <litedb/parser/nodes.h>
#include <litedb/utils/elog.h>
#include <litedb/exec/session.h>

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
  NodeDisplay(parseTree);
  MemoryContext* old = MemoryContext::SwitchTo(portalContext);

  switch (parseTree->type) {
    case T_CreateTableStmt: {
      CreateTableStmt* stmt = (CreateTableStmt*) parseTree;
      break;
    }

    default: {
      elog(ERROR, "unknown node type %d", parseTree->type);
    }
  }
  MemoryContext::SwitchTo(old);

  EndCommand();

}

void Portal::EndCommand()
{
  session->SendCommand('C', commandTag, strlen(commandTag) + 1);
}

}