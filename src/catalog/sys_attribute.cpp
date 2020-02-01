#include <litedb/catalog/sys_attribute.h>
#include <litedb/catalog/sys_class.h>
#include <litedb/catalog/sys_type.h>
#include <litedb/storage/relation.h>

namespace db {

TuplePtr SysAttribute::ToTuple(const SysAttribute& self) {
  std::vector<TupleMeta> entries;
  entries.emplace_back(INT8OID, (char*) &self.attrelid, sizeof(self.attrelid));
  entries.emplace_back(INT4OID, (char*) &self.atttypid, sizeof(self.atttypid));
  entries.emplace_back(NAMEOID, (char*) self.attname, sizeof(self.attname));
  entries.emplace_back(INT2OID, (char*) &self.attnum, sizeof(self.attnum));

  return Tuple::Construct(entries);
}

void SysAttribute::CreateEntry(TransactionPtr txn,
                               i64 attrelid,
                               i32 atttypid,
                               const char* attname,
                               i16 attnum) {
  SysAttribute entry;
  memset(&entry, 0, sizeof(entry));

  entry.attrelid = attrelid;
  entry.atttypid = atttypid;
  strcpy(entry.attname, attname);
  entry.attnum = attnum;

  TuplePtr tuple = SysAttribute::ToTuple(entry);
  RelationPtr rel = Relation::OpenTable(txn, SysAttributeRelationId);
  rel->TableAppend(*tuple);
}

void SysAttribute::InitCatalogs(TransactionPtr txn) {
  SysClass::CreateEntry(txn,
                        SysAttributeRelationId,
                        SysAttributeRelationName,
                        true,
                        RELKIND_RELATION,
                        Natts_sys_attribute);

  SysAttribute::CreateEntry(txn, SysAttributeRelationId, INT8OID, "attrelid", Anum_sys_attribute_attrelid -1);
  SysAttribute::CreateEntry(txn, SysAttributeRelationId, INT4OID, "atttypid", Anum_sys_attribute_atttypid -1);
  SysAttribute::CreateEntry(txn, SysAttributeRelationId, NAMEOID, "attname", Anum_sys_attribute_attname -1);
  SysAttribute::CreateEntry(txn, SysAttributeRelationId, INT2OID, "attnum", Anum_sys_attribute_attnum -1);
}

}