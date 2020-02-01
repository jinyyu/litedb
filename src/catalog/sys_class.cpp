#include <litedb/catalog/sys_class.h>
#include <litedb/catalog/sys_attribute.h>
#include <litedb/catalog/sys_type.h>
#include <litedb/storage/relation.h>

namespace db {

void SysClass::FromTuple(const Tuple& tuple, SysClass& self) {
  assert(tuple.columns() == Natts_sys_class);

  assert(tuple.GetType(Anum_sys_class_id - 1) == INT8OID);
  self.id = tuple.GetBasicType<i64>(Anum_sys_class_id - 1);

  assert(tuple.GetType(Anum_sys_class_relname - 1) == NAMEOID);
  Slice relname = tuple.GetSlice(Anum_sys_class_relname - 1);
  strcpy(self.relname, relname.data());

  assert(tuple.GetType(Anum_sys_class_relhasindex - 1) == BOOLOID);
  self.relhasindex = tuple.GetBasicType<bool>(Anum_sys_class_relhasindex - 1);

  assert(tuple.GetType(Anum_sys_class_relkind - 1) == CHAROID);
  self.relkind = tuple.GetBasicType<i8>(Anum_sys_class_relkind - 1);

  assert(tuple.GetType(Anum_sys_class_relnatts - 1) == INT2OID);
  self.relnatts = tuple.GetBasicType<i16>(Anum_sys_class_relnatts - 1);
}

TuplePtr SysClass::ToTuple(const SysClass& self) {
  std::vector<TupleMeta> entries;

  entries.emplace_back(INT8OID, (char*) &self.id, sizeof(self.id));
  entries.emplace_back(NAMEOID, self.relname, sizeof(self.relname));
  entries.emplace_back(BOOLOID, (char*) &self.relhasindex, sizeof(self.relhasindex));
  entries.emplace_back(CHAROID, (char*) &self.relkind, sizeof(self.relkind));
  entries.emplace_back(INT2OID, (char*) &self.relnatts, sizeof(self.relnatts));

  TuplePtr tuple = Tuple::Construct(entries);
  tuple->SetRowID(self.id);
  return tuple;
}

void SysClass::CreateEntry(TransactionPtr txn,
    i64 id,
    const char* relname,
    bool relhasindex,
    char relkind,
    i16 relnatts) {
  SysClass entry;
  memset(&entry, 0, sizeof(entry));

  entry.id = id;
  strcpy(entry.relname, relname);
  entry.relhasindex = relhasindex;
  entry.relkind = relkind;
  entry.relnatts = relnatts;

  TuplePtr tuple = SysClass::ToTuple(entry);
  RelationPtr rel = Relation::OpenTable(txn, SysClassRelationId);
  rel->TableInsert(id, *tuple);
}

void SysClass::InitCatalogs(TransactionPtr txn) {
  SysClass::CreateEntry(txn, SysClassRelationId, SysClassRelationName, true, RELKIND_RELATION, Natts_sys_class);

  SysAttribute::CreateEntry(txn, SysClassRelationId, INT8OID, "id", Anum_sys_class_id -1);
  SysAttribute::CreateEntry(txn, SysClassRelationId, NAMEOID, "relname", Anum_sys_class_relname -1);
  SysAttribute::CreateEntry(txn, SysClassRelationId, BOOLOID, "relhasindex", Anum_sys_class_relhasindex -1);
  SysAttribute::CreateEntry(txn, SysClassRelationId, CHAROID, "relkind", Anum_sys_class_relkind -1);
  SysAttribute::CreateEntry(txn, SysClassRelationId, INT2OID, "relnatts", Anum_sys_class_relnatts -1);

}

}
