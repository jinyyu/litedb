#ifndef LITEDB_UTILS_VECTOR_H_
#define LITEDB_UTILS_VECTOR_H_
#include <litedb/int.h>
#include <stdio.h>

namespace db {

struct Vector {
  u32 element_type;
  u32 element_num;
  u32 element_size;
  char* data[0];
};

u32 VectorDataSize(u32 element_num, u32 element_size);

u32 VectorSize(Vector* vector);

Vector* VectorAlloc(u32 element_type, u32 element_num, u32 element_size);

void VectorFree(Vector* vector);

template<typename T>
T VectorGet(Vector* vector, size_t index) {
  T* data = (T*) vector->data;
  return data[index];
}

template<typename T>
void VectorSet(Vector* vector, T val, size_t index) {
  T* data = (T*) vector->data;
  data[index] = val;
}

}

#endif //LITEDB_UTILS_INT_VECTOR_H_
