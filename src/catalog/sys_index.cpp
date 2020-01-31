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
  tuple->SetID(self.indexrelid);
  return tuple;
}

void SysIndex::InitCatalogs(std::vector<u64>& relations, std::vector<TuplePtr>& tuples) {
  {
    SysClass item;
    memset(&item, 0, sizeof(item));

    item.id = SysIndexRelationId;
    strcpy(item.relname, SysIndexRelationName);
    item.relhasindex = true;
    item.relkind = RELKIND_RELATION;
    item.relnatts = Natts_sys_index;

    relations.push_back(SysClassRelationId);
    tuples.push_back(SysClass::ToTuple(item));
  }

  int attnum = 0;
  {
    SysAttribute item;
    memset(&item, 0, sizeof(item));

    item.attrelid = SysIndexRelationId;
    item.atttypid = INT8OID;
    strcpy(item.attname, "indexrelid");
    item.attnum = attnum++;
    relations.push_back(SysAttributeRelationId);
    tuples.push_back(SysAttribute::ToTuple(item));
  }

  {
    SysAttribute item;
    memset(&item, 0, sizeof(item));

    item.attrelid = SysIndexRelationId;
    item.atttypid = INT8OID;
    strcpy(item.attname, "indrelid");
    item.attnum = attnum++;
    relations.push_back(SysAttributeRelationId);
    tuples.push_back(SysAttribute::ToTuple(item));
  }

  {
    SysAttribute item;
    memset(&item, 0, sizeof(item));

    item.attrelid = SysIndexRelationId;
    item.atttypid = INT2OID;
    strcpy(item.attname, "indnatts");
    item.attnum = attnum++;
    relations.push_back(SysAttributeRelationId);
    tuples.push_back(SysAttribute::ToTuple(item));
  }

  {
    SysAttribute item;
    memset(&item, 0, sizeof(item));

    item.attrelid = SysIndexRelationId;
    item.atttypid = INT2OID;
    strcpy(item.attname, "indnkeyatts");
    item.attnum = attnum++;
    relations.push_back(SysAttributeRelationId);
    tuples.push_back(SysAttribute::ToTuple(item));
  }

  {
    SysAttribute item;
    memset(&item, 0, sizeof(item));

    item.attrelid = SysIndexRelationId;
    item.atttypid = BOOLOID;
    strcpy(item.attname, "indisunique");
    item.attnum = attnum++;
    relations.push_back(SysAttributeRelationId);
    tuples.push_back(SysAttribute::ToTuple(item));
  }

  {
    SysAttribute item;
    memset(&item, 0, sizeof(item));

    item.attrelid = SysIndexRelationId;
    item.atttypid = BOOLOID;
    strcpy(item.attname, "indisprimary");
    item.attnum = attnum++;
    relations.push_back(SysAttributeRelationId);
    tuples.push_back(SysAttribute::ToTuple(item));
  }
}

}
