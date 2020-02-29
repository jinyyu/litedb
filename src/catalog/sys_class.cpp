#include <litedb/catalog/sys_class.h>
#include <litedb/catalog/sys_attribute.h>
#include <litedb/catalog/sys_type.h>
#include <litedb/storage/relation.h>
#include <litedb/utils/env.h>
#include <litedb/utils/misc.h>

namespace db {

void SysClass::FromTuple(const Tuple& tuple, SysClass& self) {
  assert(tuple.columns() == Natts_sys_class);

  assert(tuple.GetType(Anum_sys_class_relid) == INT8OID);
  self.relid = tuple.GetBasicType<i64>(Anum_sys_class_relid);

  assert(tuple.GetType(Anum_sys_class_relname) == NAMEOID);
  Slice relname = tuple.GetSlice(Anum_sys_class_relname);
  NameSetStr(&self.relname, relname.data());

  assert(tuple.GetType(Anum_sys_class_relhasindex) == BOOLOID);
  self.relhasindex = tuple.GetBasicType<bool>(Anum_sys_class_relhasindex);

  assert(tuple.GetType(Anum_sys_class_relkind) == CHAROID);
  self.relkind = tuple.GetBasicType<i8>(Anum_sys_class_relkind);

  assert(tuple.GetType(Anum_sys_class_relnatts) == INT2OID);
  self.relnatts = tuple.GetBasicType<i16>(Anum_sys_class_relnatts);
}

bool SysClass::GetCatalog(TransactionPtr txn, i64 relid, SysClass* self) {
  RelationPtr sys_class = Relation::Create(txn, SysClassRelationId);
  Slice key(&relid);
  Slice data;
  if (!sys_class->kvstore->Get(key, data)) {
    return false;
  }

  Tuple tuple((char*) data.data(), (u32) data.size());
  tuple.SetRowID(*(i64*) key.data());
  SysClass::FromTuple(tuple, *self);
  return true;
}

TuplePtr SysClass::GetSysClass(TransactionPtr txn, const char* relname) {
  TuplePtr ret;
  TuplePtr tuple;
  Name name;

  Relation* rel = Relation::OpenTable(txn, SysClassRelationId);
  ScanKey key;
  NameSetStr(&name, relname);

  ScanKey::Init(&key, Anum_sys_class_relname, BTEqualStrategyNumber, NAMEOID, Slice((char*) &name, sizeof(name)));
  SysScanDescPtr desc = SysTableBeginScan(txn, rel, sys_class_relname_index, &key, 1);
  if ((tuple = SysTableGetNext(desc)) != nullptr) {
    ret = tuple->Copy();
  }

  SysTableEndScan(desc);
  return ret;
}

TuplePtr SysClass::ToTuple(const SysClass& self) {
  std::vector<TupleMeta> entries;

  entries.emplace_back(NAMEOID, (char*) &self.relname, sizeof(self.relname));
  entries.emplace_back(BOOLOID, (char*) &self.relhasindex, sizeof(self.relhasindex));
  entries.emplace_back(CHAROID, (char*) &self.relkind, sizeof(self.relkind));
  entries.emplace_back(INT2OID, (char*) &self.relnatts, sizeof(self.relnatts));

  TuplePtr tuple = Tuple::Construct(self.relid, entries);
  return tuple;
}

i64 SysClass::CreateEntry(TransactionPtr txn,
                          i64 relid,
                          const char* relname,
                          bool relhasindex,
                          char relkind,
                          i16 relnatts) {
  SysClass entry;
  memset(&entry, 0, sizeof(entry));

  Relation* rel = Relation::OpenTable(txn, SysClassRelationId);

  if (relid > 0) {
    entry.relid = relid;
  } else {
    entry.relid = rel->TableNextID();
  }

  NameSetStr(&entry.relname, relname);
  entry.relhasindex = relhasindex;
  entry.relkind = relkind;
  entry.relnatts = relnatts;

  TuplePtr tuple = SysClass::ToTuple(entry);

  rel->TableInsert(entry.relid, *tuple);
  return entry.relid;
}

}
