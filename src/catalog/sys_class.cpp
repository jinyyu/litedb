#include <litedb/catalog/sys_class.h>
#include <litedb/storage/relation.h>

namespace db {

TuplePtr SysClass::ToTuple(const SysClass& self) {
  std::vector<Entry> entries;

  entries.emplace_back(self.relname, sizeof(self.relname));
  entries.emplace_back(&self.relhasindex, sizeof(self.relhasindex));
  entries.emplace_back(&self.relkind, sizeof(self.relkind));

  return Tuple::Construct(entries);
}

void SysClass::InsertInitData(TransactionPtr trans) {
  RelationPtr rel = Relation::OpenTable(trans, SysClassRelationId);

  {
    SysClass item;
    memset(&item, 0, sizeof(item));

    item.id = SysClassRelationId;
    strcpy(item.relname, SysClassRelationName);
    item.relhasindex = true;
    item.relkind = RELKIND_RELATION;

    TuplePtr tuple = SysClass::ToTuple(item);

    rel->InsertTuple(item.id, *tuple);
  }
}

}
