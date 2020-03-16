#include <litedb/plan/planner.h>
#include <litedb/plan/planmain.h>
#include <litedb/nodes/nodes.h>

namespace db {

static void grouping_planner(PlannerInfo* root);

PlannedStmt* Planner(Query* parse) {
  PlannedStmt* result;
  PlannerInfo* root = SubqueryPlanner(parse, nullptr, false);
  RelOptInfo* final_rel = fetch_upper_rel(root, UPPERREL_FINAL, NULL);
  Path* best_path = get_cheapest_path(final_rel);
  Plan* top_plan = create_plan(root, best_path);

  /* build the PlannedStmt result */
  result = makeNode(PlannedStmt);
  result->planTree = top_plan;
  return result;

}

PlannerInfo* SubqueryPlanner(Query* parse, PlannerInfo* parent, bool hasRecursion) {
  PlannerInfo* root;

  /* Create a PlannerInfo data structure for this subquery */
  root = makeNode(PlannerInfo);
  root->parse = parse;
  root->parent_root = parent;
  root->query_level =  parent ? parent->query_level + 1 : 1;

  grouping_planner(root);

  RelOptInfo* final_rel = fetch_upper_rel(root, UPPERREL_FINAL, NULL);

 //Make sure we've identified the cheapest Path for the final rel
  set_cheapest(final_rel);

  return root;
}

void grouping_planner(PlannerInfo* root) {

}

}
