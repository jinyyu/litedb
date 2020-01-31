#include <litedb/storage/database.h>
#include <litedb/storage/relation.h>
#include <litedb/bin/initdb.h>
#include <litedb/catalog/catalog.h>
#include <litedb/catalog/sys_class.h>
#include <gtest/gtest.h>

using namespace db;
static std::string test_db_dir = "test_relation_dir";

TEST(relation, table_scan) {
  TransactionPtr txn = CatalogDB->Begin();
  RelationPtr rel = Relation::OpenTable(txn, SysClassRelationId);

  ScanKey key;
  u64 id = SysClassRelationId;
  ScanKey::Init(&key,
      Anum_sys_class_id, BTEqualStrategyNumber,
      INT8OID, Slice((char*) &id, sizeof(id)));

  int matched = 0;
  TableScanDescPtr scan = rel->TableBeginScan(&key, 1);
  TuplePtr tuple;
  while ((tuple = rel->TableGetNext(scan)) != nullptr) {
    SysClass item;
    SysClass::FromTuple(*tuple, item);
    fprintf(stderr, "%lu, %s\n", item.id, item.relname);

    ASSERT_EQ(item.id, SysClassRelationId);
    ASSERT_TRUE(strcmp(item.relname, SysClassRelationName) == 0);
    ASSERT_EQ(item.relhasindex, true);
    ASSERT_EQ(item.relkind, RELKIND_RELATION);
    ASSERT_EQ(item.relnatts, Natts_sys_class);
    matched++;
  }

  ASSERT_EQ(matched, 1);

  rel->TableEndScan(scan);
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