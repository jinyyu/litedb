#include <litedb/catalog/sys_index.h>
#include <litedb/catalog/sys_class.h>
#include <litedb/catalog/sys_attribute.h>
#include <litedb/storage/relation.h>
#include <litedb/storage/index.h>

namespace db {

TuplePtr SysIndex::ToTuple(const SysIndex& self) {
  std::vector<TupleMeta> entries;

  entries.emplace_back(INT8OID, (char*) &self.indrelid, sizeof(self.indrelid));
  entries.emplace_back(INT2OID, (char*) &self.indnatts, sizeof(self.indnatts));
  entries.emplace_back(BOOLOID, (char*) &self.indisunique, sizeof(self.indisunique));
  entries.emplace_back(BOOLOID, (char*) &self.indisprimary, sizeof(self.indisprimary));
  entries.emplace_back(INT2VECTOROID, (char*) self.indkey, sizeof(self.indkey));

  TuplePtr tuple = Tuple::Construct(self.indexrelid, entries);
  tuple->SetRowID(self.indexrelid);
  return tuple;
}

void SysIndex::FromTuple(const Tuple& tuple, SysIndex& self) {
  assert(tuple.columns() == Natts_sys_index);

  assert(tuple.GetType(Anum_sys_index_indexrelid) == INT8OID);
  self.indexrelid = tuple.GetBasicType<i64>(Anum_sys_index_indexrelid);

  assert(tuple.GetType(Anum_sys_index_indrelid) == INT8OID);
  self.indrelid = tuple.GetBasicType<i64>(Anum_sys_index_indrelid);

  assert(tuple.GetType(Anum_sys_index_indnatts) == INT2OID);
  self.indnatts = tuple.GetBasicType<i16>(Anum_sys_index_indnatts);

  assert(tuple.GetType(Anum_sys_index_indisunique) == BOOLOID);
  self.indisunique = tuple.GetBasicType<i8>(Anum_sys_index_indisunique);

  assert(tuple.GetType(Anum_sys_index_indisprimary) == BOOLOID);
  self.indisprimary = tuple.GetBasicType<i8>(Anum_sys_index_indisprimary);

  TupleMeta meta;
  tuple.GetTupleMeta(Anum_sys_index_indkey, meta);
  assert(meta.type = INT2VECTOROID);
  assert(meta.size = sizeof(self.indkey));
  memcpy(&self.indkey, meta.data, meta.size);
}

void SysIndex::GetIndexList(TransactionPtr txn, i64 indrelid, std::vector<SysIndex>& index) {
  RelationPtr tbl = Relation::Create(txn, SysIndexRelationId);
  ScanKey key;
  ScanKey::Init(&key,
                Anum_sys_index_indrelid,
                BTEqualStrategyNumber,
                INT8OID,
                &indrelid);
  TableScanDescPtr scan = TableBeginScan(tbl.get(), &key, 1);
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
  Slice key(&indexrelid);
  Slice value;
  if (!tbl->kvstore->Get(key, value)) {
    return false;
  }

  Tuple tuple((char*) value.data(), value.size());
  FromTuple(tuple, index);
  return true;
}

void SysIndex::CreateEntry(TransactionPtr txn, const SysIndex& self) {
  RelationPtr rel = Relation::Create(txn, SysIndexRelationId);
  TuplePtr tuple = SysIndex::ToTuple(self);
  rel->TableInsert(self.indexrelid, *tuple);
}

}
