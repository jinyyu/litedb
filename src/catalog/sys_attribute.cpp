#include <litedb/catalog/sys_attribute.h>
#include <litedb/catalog/sys_class.h>
#include <litedb/catalog/sys_type.h>
#include <litedb/storage/relation.h>

namespace db {

void SysAttribute::FromTuple(const Tuple& tuple, SysAttribute& self) {
  int asdasd = tuple.columns();
  assert(asdasd == Natts_sys_attribute);

  assert(tuple.GetType(Anum_sys_attribute_attid) == INT8OID);
  self.attid = tuple.GetBasicType<i64>(Anum_sys_attribute_attid);

  assert(tuple.GetType(Anum_sys_attribute_attrelid) == INT8OID);
  self.attrelid = tuple.GetBasicType<i64>(Anum_sys_attribute_attrelid);

  assert(tuple.GetType(Anum_sys_attribute_atttypid) == INT4OID);
  self.atttypid = tuple.GetBasicType<i32>(Anum_sys_attribute_atttypid);

  assert(tuple.GetType(Anum_sys_attribute_attname) == NAMEOID);
  Slice attname = tuple.GetSlice(Anum_sys_attribute_attname);
  NameSetStr(&self.attname, attname.data());

  assert(tuple.GetType(Anum_sys_attribute_attnum) == INT2OID);
  self.attnum = tuple.GetBasicType<i16>(Anum_sys_attribute_attnum);

}

TuplePtr SysAttribute::ToTuple(const SysAttribute& self) {
  std::vector<TupleMeta> entries;
  entries.emplace_back(INT8OID, (char*) &self.attrelid, sizeof(self.attrelid));
  entries.emplace_back(INT4OID, (char*) &self.atttypid, sizeof(self.atttypid));
  entries.emplace_back(NAMEOID, (char*) &self.attname, sizeof(self.attname));
  entries.emplace_back(INT2OID, (char*) &self.attnum, sizeof(self.attnum));

  TuplePtr tuple = Tuple::Construct(self.attid, entries);
  tuple->SetRowID(self.attid);
  return tuple;
}

i64 SysAttribute::CreateEntry(TransactionPtr txn,
                              i64 attrelid,
                              i32 atttypid,
                              const char* attname,
                              i16 attnum) {
  SysAttribute entry;
  memset(&entry, 0, sizeof(entry));

  entry.attrelid = attrelid;
  entry.atttypid = atttypid;
  NameSetStr(&entry.attname, attname);
  entry.attnum = attnum;

  TuplePtr tuple = SysAttribute::ToTuple(entry);
  RelationPtr rel = Relation::Create(txn, SysAttributeRelationId);
  return rel->TableAppend(*tuple);
}

void SysAttribute::GetAttributeList(TransactionPtr txn, i64 attrelid, i16 relnatts, std::vector<SysAttribute>& atrrs) {
  Relation* attrRel = Relation::OpenTable(txn, SysAttributeRelationId);
  ScanKey keys[2];
  ScanKey::Init(&keys[0], Anum_sys_attribute_attrelid, BTEqualStrategyNumber, INT8OID, &attrelid);
  ScanKey::Init(&keys[1], Anum_sys_attribute_attnum,
                BTLessStrategyNumber, INT2OID,
                Slice((char*) &relnatts, sizeof(relnatts)));

  TuplePtr tuple;
  SysScanDescPtr scan = SysTableBeginScan(txn, attrRel, sys_attribute_attrelid_attnum_index, keys, 2);
  while ((tuple = SysTableGetNext(scan)) != nullptr) {
    SysAttribute self;
    SysAttribute::FromTuple(*tuple, self);
    atrrs.push_back(self);
  }
  SysTableEndScan(scan);
}

}