#include <litedb/nodes/pathnodes.h>

namespace db {

RelOptInfo* fetch_upper_rel(PlannerInfo* root, UpperRelationKind kind, Relids relids) {
  return nullptr;
}

Path* get_cheapest_path(RelOptInfo* rel) {
  return nullptr;
}

void set_cheapest(RelOptInfo *parent_rel) {

}

}
