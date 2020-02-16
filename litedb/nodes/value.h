#ifndef LITEDB_NODES_VALUE_H_
#define LITEDB_NODES_VALUE_H_
#include <litedb/nodes/parsenodes.h>
#include <stdlib.h>

namespace db {
struct Value {
  NodeTag type;
  union ValUnion {
    int ival;        /* machine integer */
    char* str;        /* string */
  } val;
};

#define intVal(v)        (((Value *)(v))->val.ival)
#define floatVal(v)       atof(((Value *)(v))->val.str)
#define strVal(v)        (((Value *)(v))->val.str)

Value* makeInteger(int i);
Value* makeFloat(char* numericStr);
Value* makeString(char* str);
Value* makeNull();

}

#endif //LITEDB_NODES_VALUE_H_
