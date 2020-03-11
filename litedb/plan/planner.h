#ifndef LITEDB_PLAN_PLANNER_H_
#define LITEDB_PLAN_PLANNER_H_
#include <litedb/nodes/plannodes.h>
#include <litedb/nodes/pathnodes.h>

namespace db {

PlannedStmt* Planner(Query* parse);
PlannerInfo* SubqueryPlanner(Query* parse,PlannerInfo* parent,bool hasRecursion);
}

#endif //LITEDB_PLAN_PLANNER_H_
