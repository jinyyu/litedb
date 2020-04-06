#ifndef LITEDB_PLAN_PLANMAIN_H_
#define LITEDB_PLAN_PLANMAIN_H_
#include <litedb/nodes/plannodes.h>
#include <litedb/nodes/pathnodes.h>

namespace db {

Plan* create_plan(PlannerInfo* root, Path* best_path);
RelOptInfo* query_planner(PlannerInfo* root);

}
#endif //LITEDB_PLAN_PLANMAIN_H_
