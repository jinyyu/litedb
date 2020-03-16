#ifndef LITEDB_PLAN_PLANMAIN_H_
#define LITEDB_PLAN_PLANMAIN_H_
#include <litedb/nodes/plannodes.h>
#include <litedb/nodes/pathnodes.h>

namespace db {

Plan* create_plan(PlannerInfo* root, Path* best_path);

}
#endif //LITEDB_PLAN_PLANMAIN_H_
