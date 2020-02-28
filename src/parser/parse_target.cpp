#include <litedb/parser/parse_target.h>
#include <assert.h>

namespace db {

List* TransformTargetList(ParseState* pstate, List* targetlist) {
  ListCell* o_target;
  foreach(o_target, targetlist) {
    ResTarget* res = (ResTarget*) lfirst(o_target);
    assert(res->type == T_ResTarget);

  }
}

}
