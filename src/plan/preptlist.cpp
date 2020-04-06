#include <litedb/plan/prep.h>

namespace db {

List* preprocess_targetlist(PlannerInfo* root) {
  Query* parse = root->parse;
  List* tlist;
  tlist = parse->targetList;
  return tlist;
}

}
