#include <litedb/catalog/sys_index.h>
#include <litedb/catalog/sys_class.h>
#include <litedb/catalog/sys_attribute.h>
#include <litedb/storage/relation.h>
namespace db {

TuplePtr SysIndex::ToTuple(const SysIndex& self) {
  std::vector<TupleMeta> entries;

  entries.emplace_back(INT8OID, (char*) &self.indexrelid, sizeof(self.indexrelid));
  entries.emplace_back(INT8OID, (char*) &self.indrelid, sizeof(self.indrelid));
  entries.emplace_back(INT2OID, (char*) &self.indnatts, sizeof(self.indnatts));
  entries.emplace_back(INT2OID, (char*) &self.indnkeyatts, sizeof(self.indnkeyatts));
  entries.emplace_back(BOOLOID, (char*) &self.indisunique, sizeof(self.indisunique));
  entries.emplace_back(BOOLOID, (char*) &self.indisprimary, sizeof(self.indisprimary));
  entries.emplace_back(INT2VECTOROID, (char*) &self.indkey, sizeof(self.indkey));

  TuplePtr tuple = Tuple::Construct(entries);
  tuple->SetRowID(self.indexrelid);
  return tuple;
}

void SysIndex::FromTuple(const Tuple& tuple, SysIndex& self) {
  assert(tuple.columns() == Natts_sys_index);

  assert(tuple.GetType(Anum_sys_index_indexrelid - 1) == INT8OID);
  self.indexrelid = tuple.GetBasicType<i64>(Anum_sys_index_indexrelid - 1);

  assert(tuple.GetType(Anum_sys_index_indrelid - 1) == INT8OID);
  self.indrelid = tuple.GetBasicType<i64>(Anum_sys_index_indrelid - 1);

  assert(tuple.GetType(Anum_sys_index_indnatts - 1) == INT2OID);
  self.indnatts = tuple.GetBasicType<i16>(Anum_sys_index_indnatts - 1);

  assert(tuple.GetType(Anum_sys_index_indnkeyatts - 1) == INT2OID);
  self.indnkeyatts = tuple.GetBasicType<i16>(Anum_sys_index_indnkeyatts - 1);

  assert(tuple.GetType(Anum_sys_index_indisunique - 1) == BOOLOID);
  self.indisunique = tuple.GetBasicType<i8>(Anum_sys_index_indisunique - 1);

  assert(tuple.GetType(Anum_sys_index_indisprimary - 1) == BOOLOID);
  self.indisprimary = tuple.GetBasicType<i8>(Anum_sys_index_indisprimary - 1);

  TupleMeta meta;
  tuple.Get(Anum_sys_index_indkey - 1, meta);
  assert(meta.type = INT2VECTOROID);
  assert(meta.size = sizeof(Vector));
  memcpy(&self.indkey, meta.data, meta.size);
}

void SysIndex::GetIndexList(TransactionPtr txn, i64 indrelid, std::vector<SysIndex>& index) {
  RelationPtr tbl = Relation::Create(txn, SysIndexRelationId);
  ScanKey key;
  ScanKey::Init(&key,
                Anum_sys_index_indrelid,
                BTEqualStrategyNumber,
                INT8OID,
                Slice((char*) &indrelid, sizeof(indrelid)));
  TableScanDescPtr scan = TableBeginScan(tbl, &key, 1);
  TuplePtr tuple;
  while ((tuple = TableGetNext(scan)) != nullptr) {
    i64 relid = tuple->GetBasicType<i64>(Anum_sys_index_indrelid - 1);

    if (relid == indrelid) {
      SysIndex indexTup;
      SysIndex::FromTuple(*tuple, indexTup);
      index.push_back(indexTup);
    }
  }
  TableEndScan(scan);
}

bool SysIndex::GetIndexTuple(TransactionPtr txn, i64 indexrelid, SysIndex& index) {
  RelationPtr tbl = Relation::Create(txn, SysIndexRelationId);
  Slice key((char*) &indexrelid, sizeof(indexrelid));
  Slice value;
  if (!tbl->kvstore->Get(key, value)) {
    return false;
  }

  Tuple tuple((char*) value.data(), value.size());
  FromTuple(tuple, index);
  return true;
}

void SysIndex::CreateEntry(TransactionPtr txn, const SysIndex& self) {
  RelationPtr rel = Relation::OpenTable(txn, SysIndexRelationId);
  TuplePtr tuple = SysIndex::ToTuple(self);
  rel->TableInsert(self.indexrelid, *tuple);
}

}
