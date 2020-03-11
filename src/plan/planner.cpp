#include <litedb/plan/planner.h>

namespace db {

PlannedStmt* Planner(Query* parse) {
  PlannerInfo* root = SubqueryPlanner(parse, nullptr, false);

}

PlannerInfo* SubqueryPlanner(Query* parse, PlannerInfo* parent, bool hasRecursion) {

}

}
