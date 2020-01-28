#include <litedb/catalog/sys_attribute.h>
#include <litedb/catalog/sys_class.h>

namespace db {

TuplePtr SysAttribute::ToTuple(const SysAttribute& self) {
  std::vector<Slice> entries;
  entries.emplace_back((char*)&self.attrelid, sizeof(self.attrelid));
  entries.emplace_back((char*)&self.atttypid, sizeof(self.atttypid));
  entries.emplace_back((char*)self.attname, sizeof(self.attname));
  entries.emplace_back((char*)&self.attnum, sizeof(self.attnum));

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

    relations.push_back(SysAttributeRelationId);
    tuples.push_back(SysClass::ToTuple(item));
  }

}

}