#ifndef LITEDB_STORAGE_RELATION_H_
#define LITEDB_STORAGE_RELATION_H_
#include <memory>
#include <litedb/int.h>
#include <litedb/storage/database.h>
#include <litedb/storage/tuple.h>
#include <litedb/storage/scan_key.h>

namespace db {
class TableScanDesc;
typedef std::shared_ptr<TableScanDesc> TableScanDescPtr;

class Relation;
typedef std::shared_ptr<Relation> RelationPtr;

class Relation {
 public:
  static RelationPtr OpenTable(TransactionPtr tran, u64 id);

  static RelationPtr OpenIndex(TransactionPtr tran, u64 id);

  ~Relation() = default;

  Table* GetTable() {
    return table_;
  }

  /*
   * insert a tuple
   */
  void TableInsert(u64 id, const Tuple& tuple);

  /*
   * append a tuple
   */
  i64 TableAppend(const Tuple& tuple);

  i64 TableNextID();

  TableScanDescPtr TableBeginScan(ScanKey* scanKey, int nkeys);
  TuplePtr TableGetNext(TableScanDescPtr& scan);
  void TableEndScan(TableScanDescPtr& scan);

 private:
  explicit Relation(Table* table);

  Table* table_;
  char relkind_;
};

}
#endif //LITEDB_STORAGE_RELATION_H_
