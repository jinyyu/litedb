#ifndef LITEDB_STORAGE_RELATION_H_
#define LITEDB_STORAGE_RELATION_H_
#include <memory>
#include <litedb/int.h>
#include <litedb/storage/database.h>
#include <litedb/storage/tuple.h>
#include <litedb/storage/scan_key.h>
#include <litedb/catalog/sys_index.h>
#include <litedb/catalog/sys_class.h>

namespace db {
class Relation;
typedef std::shared_ptr<Relation> RelationPtr;

class Relation {
 public:
  static RelationPtr Create(TransactionPtr txn, i64 id);

  static Relation* OpenTable(TransactionPtr txn, i64 id);

  static Relation* OpenIndex(TransactionPtr txn, i64 id);

  ~Relation() = default;

  /*
   * insert a tuple
   */
  void TableInsert(i64 id, const Tuple& tuple);

  /*
   * append a tuple
   */
  i64 TableAppend(const Tuple& tuple);

  i64 TableNextID();

  explicit Relation(KVStore* table);

  i64 relid;                        /*relation id*/
  KVStore* kvstore;
  SysClass rd_rel;                  /* RELATION tuple */
  std::vector<SysIndex> rd_index;   /* list of indexes on relation */
};

class TableScanDesc;
typedef std::shared_ptr<TableScanDesc> TableScanDescPtr;
TableScanDescPtr TableBeginScan(Relation* rel, ScanKey* scanKey, int nkeys);
TuplePtr TableGetNext(TableScanDescPtr scan);
void TableEndScan(TableScanDescPtr& scan);

class SysScanDesc;
typedef std::shared_ptr<SysScanDesc> SysScanDescPtr;
SysScanDescPtr SysTableBeginScan(TransactionPtr txn,
                                 Relation* tableRel,
                                 i64 indexId,
                                 ScanKey* scanKey, int nkeys);
TuplePtr SysTableGetNext(SysScanDescPtr scan);
void SysTableEndScan(SysScanDescPtr& scan);

}
#endif //LITEDB_STORAGE_RELATION_H_
