#ifndef LITEDB_NODES_PATHNODES_H_
#define LITEDB_NODES_PATHNODES_H_
#include <litedb/nodes/nodes.h>
#include <litedb/nodes/parsenodes.h>
#include <litedb/nodes/pathnodes.h>
#include <litedb/utils/bitmapset.h>

namespace db {

struct Path {

};

/*
 * Relids
 *		Set of relation identifiers (indexes into the rangetable).
 */
typedef Bitmapset* Relids;

/*
 * This enum identifies the different types of "upper" (post-scan/join)
 * relations that we might deal with during planning.
 */
typedef enum UpperRelationKind {
  UPPERREL_SETOP,               /* result of UNION/INTERSECT/EXCEPT, if any */
  UPPERREL_GROUP_AGG,           /* result of grouping/aggregation, if any */
  UPPERREL_WINDOW,              /* result of window functions, if any */
  UPPERREL_DISTINCT,            /* result of "SELECT DISTINCT", if any */
  UPPERREL_ORDERED,             /* result of ORDER BY, if any */
  UPPERREL_FINAL                /* result of any remaining top-level actions */
} UpperRelationKind;

struct PlannerInfo {
  NodeTag type;
  Query* parse;                     /* the Query being planned */
  i32 query_level;                  /* 1 at the outermost Query */
  PlannerInfo* parent_root;         /* NULL at outermost Query */
  Path* non_recursive_path;         /* a path for non-recursive term */

  List* processed_tlist;            /* The fully-processed targetlist is kept here */

  List* upper_rels[UPPERREL_FINAL + 1]; /* upper-rel RelOptInfos */

};

struct RelOptInfo {

};

RelOptInfo* fetch_upper_rel(PlannerInfo* root, UpperRelationKind kind, Relids relids);
Path* get_cheapest_path(RelOptInfo* rel);
void set_cheapest(RelOptInfo* parent_rel);

}

#endif //LITEDB_NODES_PATHNODES_H_
