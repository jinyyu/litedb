#include <litesql/portal.h>
#include <litesql/nodes.h>
#include <litesql/elog.h>
#include <litesql/table.h>
namespace db {

void Portal::Run() {
  NodeDisplay(parseTree);
  MemoryContext* old = MemoryContext::SwitchTo(portalContext);

  switch (parseTree->type) {
    case T_CreateTableStmt: {
      CreateTableStmt* stmt = (CreateTableStmt*) parseTree;
      ProcessCreateTableStmt(stmt);
      break;
    }

    default:{
      eReport(ERROR, "unknown node type %d", parseTree->type);
    }

  }

  MemoryContext::SwitchTo(old);
}

}