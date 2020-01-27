#include <litedb/catalog/sys_class.h>
#include <litedb/catalog/sys_attribute.h>
#include <litedb/storage/relation.h>

namespace db {

TuplePtr SysClass::ToTuple(const SysClass& self) {
  std::vector<Entry> entries;

  entries.emplace_back(&self.id, sizeof(self.id));
  entries.emplace_back(self.relname, sizeof(self.relname));
  entries.emplace_back(&self.relhasindex, sizeof(self.relhasindex));
  entries.emplace_back(&self.relkind, sizeof(self.relkind));

  return Tuple::Construct(entries);
}


void SysClass::InitCatalogs(std::vector<u64>& relations, std::vector<TuplePtr>& tuples)
{
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

  {
    SysClass item;
    memset(&item, 0, sizeof(item));

    item.id = SysAttributeRelationId;
    strcpy(item.relname, SysAttributeRelationName);
    item.relhasindex = true;
    item.relkind = RELKIND_RELATION;

    relations.push_back(SysAttributeRelationId);
    tuples.push_back(SysClass::ToTuple(item));
  }
}

}
