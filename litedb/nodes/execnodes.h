#ifndef LITEDB_NODES_EXECNODES_H_
#define LITEDB_NODES_EXECNODES_H_
#include <litedb/int.h>

namespace db {

struct IndexInfo {
  int ii_NumIndexKeyAttrs;    /* number of key columns in index */
  i16 ii_IndexAttrNumbers[INDEX_MAX_KEYS];
  bool ii_Unique;
};

}

#endif //LITEDB_NODES_EXECNODES_H_
