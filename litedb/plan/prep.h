#ifndef LITEDB_PLAN_PREP_H_
#define LITEDB_PLAN_PREP_H_
#include <litedb/utils/list.h>
#include <litedb/nodes/pathnodes.h>

namespace db {

List* preprocess_targetlist(PlannerInfo* root);

}

#endif //LITEDB_PLAN_PREP_H_
