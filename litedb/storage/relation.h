#ifndef LITEDB_STORAGE_RELATION_H_
#define LITEDB_STORAGE_RELATION_H_
#include <memory>
#include <litedb/int.h>
#include <litedb/storage/database.h>
#include <litedb/storage/tuple.h>
#include <litedb/storage/scan_key.h>

namespace db {
class Relation;
typedef std::shared_ptr<Relation> RelationPtr;

class Relation {
 public:
  static RelationPtr Create(TransactionPtr tran, u64 id);

  static RelationPtr OpenTable(TransactionPtr tran, u64 id);

  static RelationPtr OpenIndex(TransactionPtr tran, u64 id);

  ~Relation() = default;

  /*
   * insert a tuple
   */
  void TableInsert(u64 id, const Tuple& tuple);

  /*
   * append a tuple
   */
  i64 TableAppend(const Tuple& tuple);

  i64 TableNextID();

  explicit Relation(KVStore* table);

  i64 relid;
  KVStore* kvstore;
  char relkind;

};

class TableScanDesc;
typedef std::shared_ptr<TableScanDesc> TableScanDescPtr;
TableScanDescPtr TableBeginScan(RelationPtr rel, ScanKey* scanKey, int nkeys);
TuplePtr TableGetNext(TableScanDescPtr scan);
void TableEndScan(TableScanDescPtr& scan);


class SysScanDesc;
typedef std::shared_ptr<SysScanDesc> SysScanDescPtr;
SysScanDescPtr SysTableBeginScan(RelationPtr tableRel,
                                 u64 indexId,
                                 bool indexOK,
                                 ScanKey* scanKey, int nkeys);
TuplePtr SysTableGetNext(SysScanDescPtr scan);
void SysTableEndScan(TableScanDescPtr& scan);

}
#endif //LITEDB_STORAGE_RELATION_H_
