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
  Relation* rel = Relation::OpenTable(txn, SysClassRelationId);

  ScanKey key;
  i64 id = SysClassRelationId;
  ScanKey::Init(&key,
                Anum_sys_class_relid, BTEqualStrategyNumber,
                INT8OID, &id);

  int matched = 0;
  TableScanDescPtr scan = TableBeginScan(rel, &key, 1);
  TuplePtr tuple;
  while ((tuple = TableGetNext(scan)) != nullptr) {
    SysClass item;
    SysClass::FromTuple(*tuple, item);
    fprintf(stderr, "%lu, %s\n", item.relid, item.relname.data);

    ASSERT_EQ(item.relid, SysClassRelationId);
    ASSERT_TRUE(strcmp(item.relname.data, SysClassRelationName) == 0);
    ASSERT_EQ(item.relhasindex, true);
    ASSERT_EQ(item.relkind, RELKIND_RELATION);
    ASSERT_EQ(item.relnatts, Natts_sys_class);
    matched++;
  }

  ASSERT_EQ(matched, 1);

  TableEndScan(scan);
}

TEST(relation, table_scan2) {
  TransactionPtr txn = CatalogDB->Begin();
  Relation* rel = Relation::OpenTable(txn, SysClassRelationId);


  int matched = 0;
  TableScanDescPtr scan = TableBeginScan(rel, nullptr, 0);
  TuplePtr tuple;
  while ((tuple = TableGetNext(scan)) != nullptr) {
    SysClass item;
    SysClass::FromTuple(*tuple, item);
    fprintf(stderr, "%lu, %s\n", item.relid, item.relname.data);
    matched++;
  }
  TableEndScan(scan);
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