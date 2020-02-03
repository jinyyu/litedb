#include <litedb/utils/vector.h>
#include <litedb/utils/env.h>

namespace db {

u32 VectorDataSize(u32 element_num, u32 element_size) {
  return element_num * element_size;
}

u32 VectorSize(Vector* vector) {
  return VectorDataSize(vector->element_num, vector->element_size) + sizeof(Vector);
}

Vector* VectorAlloc(u32 element_type, u32 element_num, u32 element_size) {
  u32 vLen = VectorDataSize(element_num, element_size);
  Vector* vector = (Vector*) SessionEnv->Malloc0(sizeof(Vector) + vLen);
  vector->element_type = element_type;
  vector->element_num = element_num;
  vector->element_size = element_size;
  return vector;
}

void VectorFree(Vector* vector) {
  SessionEnv->Free((void*) vector);
}

}
