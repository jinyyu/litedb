#ifndef LITEDB_UTILS_LIST_H_
#define LITEDB_UTILS_LIST_H_
#include <list>
#include <litedb/utils/env.h>

namespace db {

template<typename T>
struct List : public Object {
  explicit List()
      : Object() {
  }

  explicit List(T* t)
      : Object() {
    list.push_back(t);
  }

  explicit List(T* t1, T* t2)
      : Object() {
    list.push_back(t1);
    list.push_back(t2);
  }

  void Append(T* t) {
    list.push_back(t);
  }

  ~List() final {}
  std::list<T*> list;
};

}
#endif //LITEDB_UTILS_LIST_H_
