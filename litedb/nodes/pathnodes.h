#ifndef LITEDB_NODES_PATHNODES_H_
#define LITEDB_NODES_PATHNODES_H_
#include <litedb/nodes/nodes.h>

namespace db {

struct PlannerInfo {
  NodeTag type;
  Query* parse;                    /* the Query being planned */
  PlannerInfo* parent_root;        /* NULL at outermost Query */
  struct Path* non_recursive_path; /* a path for non-recursive term */


};

}

#endif //LITEDB_NODES_PATHNODES_H_
