#include <litedb/catalog/sys_attribute.h>
#include <litedb/catalog/sys_class.h>
#include <litedb/catalog/sys_type.h>

namespace db {

TuplePtr SysAttribute::ToTuple(const SysAttribute& self) {
  std::vector<TupleMeta> entries;
  entries.emplace_back(INT8OID, (char*) &self.attrelid, sizeof(self.attrelid));
  entries.emplace_back(INT4OID, (char*) &self.atttypid, sizeof(self.atttypid));
  entries.emplace_back(NAMEOID, (char*) self.attname, sizeof(self.attname));
  entries.emplace_back(INT2OID, (char*) &self.attnum, sizeof(self.attnum));

  return Tuple::Construct(entries);
}

void SysAttribute::InitCatalogs(std::vector<u64>& relations, std::vector<TuplePtr>& tuples) {
  {
    SysClass item;
    memset(&item, 0, sizeof(item));

    item.id = SysAttributeRelationId;
    strcpy(item.relname, SysAttributeRelationName);
    item.relhasindex = true;
    item.relkind = RELKIND_RELATION;

    relations.push_back(SysClassRelationId);
    tuples.push_back(SysClass::ToTuple(item));
  }

  int attnum = 0;
  {
    SysAttribute item;
    memset(&item, 0, sizeof(item));

    item.attrelid = SysAttributeRelationId;
    item.atttypid = INT8OID;
    strcpy(item.attname, "attrelid");
    item.attnum = attnum++;
    relations.push_back(SysAttributeRelationId);
    tuples.push_back(SysAttribute::ToTuple(item));
  }

  {
    SysAttribute item;
    memset(&item, 0, sizeof(item));

    item.attrelid = SysAttributeRelationId;
    item.atttypid = INT4OID;
    strcpy(item.attname, "atttypid");
    item.attnum = attnum++;
    relations.push_back(SysAttributeRelationId);
    tuples.push_back(SysAttribute::ToTuple(item));
  }

  {
    SysAttribute item;
    memset(&item, 0, sizeof(item));

    item.attrelid = SysAttributeRelationId;
    item.atttypid = NAMEOID;
    strcpy(item.attname, "attname");
    item.attnum = attnum++;
    relations.push_back(SysAttributeRelationId);
    tuples.push_back(SysAttribute::ToTuple(item));
  }

  {
    SysAttribute item;
    memset(&item, 0, sizeof(item));

    item.attrelid = SysAttributeRelationId;
    item.atttypid = INT2OID;
    strcpy(item.attname, "attnum");
    item.attnum = attnum++;
    relations.push_back(SysAttributeRelationId);
    tuples.push_back(SysAttribute::ToTuple(item));
  }

}

}