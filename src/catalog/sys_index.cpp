#include <litedb/catalog/sys_index.h>
#include <litedb/catalog/sys_class.h>
#include <litedb/catalog/sys_attribute.h>

namespace db {

TuplePtr SysIndex::ToTuple(const SysIndex& self) {
  std::vector<TupleMeta> entries;

  entries.emplace_back(INT8OID, (char*) &self.indexrelid, sizeof(self.indexrelid));
  entries.emplace_back(INT8OID, (char*) &self.indrelid, sizeof(self.indrelid));
  entries.emplace_back(INT2OID, (char*) &self.indnatts, sizeof(self.indnatts));
  entries.emplace_back(INT2OID, (char*) &self.indnkeyatts, sizeof(self.indnkeyatts));
  entries.emplace_back(BOOLOID, (char*) &self.indisunique, sizeof(self.indisunique));
  entries.emplace_back(BOOLOID, (char*) &self.indisprimary, sizeof(self.indisprimary));

  TuplePtr tuple = Tuple::Construct(entries);
  tuple->SetRowID(self.indexrelid);
  return tuple;
}

void SysIndex::InitCatalogs(TransactionPtr txn) {
  SysClass::CreateEntry(txn, SysIndexRelationId, SysIndexRelationName, true, RELKIND_RELATION, Natts_sys_index);

  SysAttribute::CreateEntry(txn, SysIndexRelationId, INT8OID, "indexrelid", Anum_sys_index_indexrelid - 1);
  SysAttribute::CreateEntry(txn, SysIndexRelationId, INT8OID, "indrelid", Anum_sys_index_indrelid - 1);
  SysAttribute::CreateEntry(txn, SysIndexRelationId, INT2OID, "indnatts", Anum_sys_index_indnatts - 1);
  SysAttribute::CreateEntry(txn, SysIndexRelationId, INT2OID, "indnkeyatts", Anum_sys_index_indnkeyatts - 1);
  SysAttribute::CreateEntry(txn, SysIndexRelationId, BOOLOID, "indisunique", Anum_sys_index_indisunique - 1);
  SysAttribute::CreateEntry(txn, SysIndexRelationId, BOOLOID, "indisprimary", Anum_sys_index_indisprimary - 1);
}

}
