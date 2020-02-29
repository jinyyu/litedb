#include <litedb/storage/database.h>
#include <litedb/storage/relation.h>
#include <litedb/bin/initdb.h>
#include <litedb/catalog/catalog.h>
#include <litedb/catalog/sys_class.h>
#include <litedb/storage/index.h>
#include <litedb/utils/misc.h>
#include <gtest/gtest.h>

using namespace db;
static std::string test_db_dir = "test_index_dir";

TEST(relation, index_scan) {
  TransactionPtr txn = CatalogDB->Begin();
  Relation* rel = Relation::OpenTable(txn, SysClassRelationId);

  Relation* index = Relation::OpenIndex(txn, 1227);

  IndexInfo info;
  memset(&info, 0, sizeof(info));
  info.ii_Unique = true;
  info.ii_NumIndexKeyAttrs = 1;
  info.ii_IndexAttrNumbers[0] = Anum_sys_class_relname;

  IndexAmBuild(rel, index, &info);
  txn->Commit();

  txn = CatalogDB->Begin();
  rel = Relation::OpenTable(txn, SysClassRelationId);
  index = Relation::OpenIndex(txn, 1227);

  ScanKey key;
  Name name;
  NameSetStr(&name, SysClassRelationName);

  ScanKey::Init(&key, Anum_sys_class_relname, BTEqualStrategyNumber, NAMEOID, NameGetSlice(&name));
  IndexScanDescPtr desc = IndexBeginScan(rel, index, &key, 1);
  TuplePtr tuple;
  int matched = 0;
  while ((tuple = IndexGetNext(desc)) != nullptr) {
    TupleMeta meta;
    tuple->GetTupleMeta(Anum_sys_class_relname, meta);
    fprintf(stderr, "type = %d, size = %d, str = %s\n", meta.type, meta.size, meta.data);
    ++matched;
  }
  IndexEndScan(desc);
  ASSERT_EQ(matched, 1);
}

TEST(relation, sys_scan) {
  TransactionPtr txn = CatalogDB->Begin();
  Relation* rel = Relation::OpenTable(txn, SysClassRelationId);

  ScanKey key;
  i64 id = SysClassRelationId;
  ScanKey::Init(&key, Anum_sys_class_relid, BTEqualStrategyNumber, INT8OID, &id);

  SysScanDescPtr desc = SysTableBeginScan(txn, rel, 0, &key, 1);
  TuplePtr tuple;
  int matched = 0;
  while ((tuple = SysTableGetNext(desc)) != nullptr) {
    TupleMeta meta;
    tuple->GetTupleMeta(Anum_sys_class_relname, meta);
    fprintf(stderr, "type = %d, size = %d, str = %s\n", meta.type, meta.size, meta.data);
    ++matched;
  }
  SysTableEndScan(desc);
  ASSERT_EQ(matched, 1);
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