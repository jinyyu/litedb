#include <litedb/utils/vector.h>
#include <litedb/utils/env.h>

namespace db {

void VectorInit(Vector* vec, u32 element_type, u32 element_num, u32 element_size) {
  memset(vec, 0, sizeof(Vector));
  vec->element_type = element_type;
  vec->element_num = element_num;
  vec->element_size = element_type;
}

}
