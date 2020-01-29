#include <litedb/catalog/sys_class.h>
#include <litedb/catalog/sys_attribute.h>
#include <litedb/catalog/sys_type.h>
#include <litedb/storage/relation.h>

namespace db {

TuplePtr SysClass::ToTuple(const SysClass& self) {
  std::vector<TupleMeta> entries;

  entries.emplace_back(INT8OID, (char*) &self.id, sizeof(self.id));
  entries.emplace_back(NAMEOID, self.relname, sizeof(self.relname));
  entries.emplace_back(BOOLOID, (char*) &self.relhasindex, sizeof(self.relhasindex));
  entries.emplace_back(CHAROID, (char*) &self.relkind, sizeof(self.relkind));

  TuplePtr tuple = Tuple::Construct(entries);
  tuple->SetID(self.id);
  return tuple;
}

void SysClass::InitCatalogs(std::vector<u64>& relations, std::vector<TuplePtr>& tuples) {
  {
    SysClass item;
    memset(&item, 0, sizeof(item));

    item.id = SysClassRelationId;
    strcpy(item.relname, SysClassRelationName);
    item.relhasindex = true;
    item.relkind = RELKIND_RELATION;

    relations.push_back(SysClassRelationId);
    tuples.push_back(SysClass::ToTuple(item));
  }

  int attnum = 0;
  {
    SysAttribute item;
    memset(&item, 0, sizeof(item));

    item.attrelid = SysClassRelationId;
    item.atttypid = INT8OID;
    strcpy(item.attname, "id");
    item.attnum = attnum++;
    relations.push_back(SysAttributeRelationId);
    tuples.push_back(SysAttribute::ToTuple(item));
  }

  {
    SysAttribute item;
    memset(&item, 0, sizeof(item));

    item.attrelid = SysClassRelationId;
    item.atttypid = NAMEOID;
    strcpy(item.attname, "relname");
    item.attnum = attnum++;
    relations.push_back(SysAttributeRelationId);
    tuples.push_back(SysAttribute::ToTuple(item));
  }

  {
    SysAttribute item;
    memset(&item, 0, sizeof(item));

    item.attrelid = SysClassRelationId;
    item.atttypid = BOOLOID;
    strcpy(item.attname, "relhasindex");
    item.attnum = attnum++;
    relations.push_back(SysAttributeRelationId);
    tuples.push_back(SysAttribute::ToTuple(item));
  }

  {
    SysAttribute item;
    memset(&item, 0, sizeof(item));

    item.attrelid = SysClassRelationId;
    item.atttypid = CHAROID;
    strcpy(item.attname, "relkind");
    item.attnum = attnum++;
    relations.push_back(SysAttributeRelationId);
    tuples.push_back(SysAttribute::ToTuple(item));
  }
}

}
