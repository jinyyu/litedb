#include <litedb/storage/database.h>
#include <litedb/storage/relation.h>
#include <litedb/bin/initdb.h>
#include <litedb/catalog/catalog.h>
#include <litedb/catalog/sys_class.h>
#include <litedb/catalog/sys_attribute.h>
#include <litedb/storage/index.h>
#include <litedb/utils/misc.h>
#include <gtest/gtest.h>

using namespace db;
static std::string test_db_dir = "test_catalog_dir";

TEST(relation, show_sys_index) {
  TransactionPtr txn = CatalogDB->Begin();
  Relation* rel = Relation::OpenTable(txn, SysIndexRelationId);

  SysScanDescPtr desc = SysTableBeginScan(txn, rel, 0, nullptr, 0);
  TuplePtr tuple;
  fprintf(stderr, "==================================\n");
  while ((tuple = SysTableGetNext(desc)) != nullptr) {
    SysIndex ind;
    SysIndex::FromTuple(*tuple, ind);
    fprintf(stderr, "(indexrelid=%lu, indrelid=%lu, indnatts=%d, indisunique=%d, indisprimary=%d, indnatts=[",
            ind.indexrelid, ind.indrelid, ind.indnatts, ind.indisunique, ind.indisprimary);
    for (int i = 0; i < ind.indnatts; ++i) {
      fprintf(stderr, " %d ", ind.indkey[i]);
    }
    fprintf(stderr, "]\n");
  }
  SysTableEndScan(desc);
}

TEST(relation, show_sys_class) {
  TransactionPtr txn = CatalogDB->Begin();
  Relation* rel = Relation::OpenTable(txn, SysClassRelationId);

  SysScanDescPtr desc = SysTableBeginScan(txn, rel, 0, nullptr, 0);
  TuplePtr tuple;
  fprintf(stderr, "==================================\n");
  while ((tuple = SysTableGetNext(desc)) != nullptr) {
    SysClass item;
    SysClass::FromTuple(*tuple, item);
    fprintf(stderr, "(relid=%lu, relname=%s, relhasindex=%d, relkind=%c, relnatts=%d\n",
            item.relid, item.relname.data, item.relhasindex, item.relkind, item.relnatts);
  }
  SysTableEndScan(desc);
}

TEST(relation, show_sys_attribute) {
  TransactionPtr txn = CatalogDB->Begin();
  Relation* rel = Relation::OpenTable(txn, SysAttributeRelationId);

  SysScanDescPtr desc = SysTableBeginScan(txn, rel, 0, nullptr, 0);
  TuplePtr tuple;
  fprintf(stderr, "==================================\n");
  while ((tuple = SysTableGetNext(desc)) != nullptr) {
    SysAttribute item;
    SysAttribute::FromTuple(*tuple, item);
    fprintf(stderr, "(attid=%lu, attrelid=%lu, atttypid=%d, attname=%s, attnum=%d\n",
            item.attid, item.attrelid, item.atttypid, item.attname.data, item.attnum);
  }
  SysTableEndScan(desc);
}

TEST(relation, show_sys_attribute2) {

  fprintf(stderr, "================sys_class==================\n");

  TransactionPtr txn = CatalogDB->Begin();
  std::vector<SysAttribute> attrs;
  SysAttribute::GetAttributeList(txn, SysClassRelationId, Natts_sys_class, attrs);

  for (SysAttribute& attr : attrs) {
    fprintf(stderr, "(attid=%lu, attrelid=%lu, atttypid=%d, attname=%s, attnum=%d\n",
            attr.attid, attr.attrelid, attr.atttypid, attr.attname.data, attr.attnum);
  }
}

int main(int argc, char* argv[]) {
  system(("rm -rf " + test_db_dir).c_str());

  InitDBMain(test_db_dir.c_str());

  CatalogDB = Database::Open("catalog");

  testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();

  Database::Close(CatalogDB);

  system(("rm -rf " + test_db_dir).c_str());
  return ret;
}