#ifndef LITEDB_STORAGE_RELATION_H_
#define LITEDB_STORAGE_RELATION_H_
#include <memory>
#include <litedb/int.h>
#include <litedb/storage/database.h>
#include <litedb/storage/tuple.h>

namespace db {

class Relation;
typedef std::shared_ptr<Relation> RelationPtr;

class Relation {
 public:
  static RelationPtr OpenTable(TransactionPtr tran, u64 id);

  static RelationPtr OpenIndex(TransactionPtr tran, u64 id);

  ~Relation() {}

  Table* GetTable() {
    return table_;
  }

  void InsertTuple(u64 id, const Tuple& tuple);

  u64 Append(const Tuple& tuple);

 private:
  explicit Relation(Table* table);

  Table* table_;
};

}
#endif //LITEDB_STORAGE_RELATION_H_
