#include <litesql/portal.h>
#include <litesql/nodes.h>
#include <litesql/elog.h>
#include <litesql/table.h>
#include <litesql/session.h>

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
      ProcessCreateTableStmt(stmt);
      break;
    }

    default: {
      eReport(ERROR, "unknown node type %d", parseTree->type);
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