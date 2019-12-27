#ifndef LITEDB_LITEDB_UTILS_LIST_H_
#define LITEDB_LITEDB_UTILS_LIST_H_
#include <list>
#include <litedb/utils/memctx.h>

namespace db {

template<typename T>
struct List : public Object {
  explicit List()
      : Object(CurTransactionContext) {
  }

  explicit List(T* t)
      : Object(CurTransactionContext) {
    list.push_back(t);
  }

  explicit List(T* t1, T* t2)
      : Object(CurTransactionContext) {
    list.push_back(t1);
    list.push_back(t2);
  }

  void Append(T* t) {
    list.push_back(t);
  }

  ~List() override {}
  std::list<T*> list;
};

}
#endif //LITEDB_LITEDB_UTILS_LIST_H_
