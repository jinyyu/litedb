#include <litedb/storage/database.h>
#include <litedb/storage/relation.h>
#include <gtest/gtest.h>

using namespace db;
static Database* test_db;
static std::string test_db_dir = "test_database_dir";

TEST(database, commit) {
  TransactionPtr txn = test_db->Begin();
  KVStore* tbl = txn->Open("test1", MDB_CREATE);
  Slice key("test key");

  Slice value("test_data");
  tbl->Put(key, value, 0);
  txn->Commit();

  txn = test_db->Begin();
  tbl = txn->Open("test1", MDB_CREATE);

  Slice value1;
  tbl->Get(key, value1);
  ASSERT_EQ(key.to_string(), "test key");

  ASSERT_EQ(value1.to_string(), std::string("test_data"));
}

TEST(database, cursur_get) {
  TransactionPtr txn = test_db->Begin();
  KVStore* tbl = txn->Open("test2", MDB_CREATE);

  Cursor* cursor = tbl->Open();
  Slice key;
  Slice value;
  ASSERT_EQ(cursor->Get(key, value, MDB_LAST), false);
  ASSERT_EQ(cursor->Put("key1", "value1", MDB_APPEND), true);

  ASSERT_EQ(cursor->Get(key, value, MDB_LAST), true);
  ASSERT_EQ(key.to_string(), "key1");
  ASSERT_EQ(value.to_string(), "value1");
  txn->Commit();
}

TEST(relation, insert) {
  std::vector<TupleMeta> entries;
  u8 v1 = std::numeric_limits<u8>::max();
  entries.emplace_back(100, (char*) &v1, sizeof(v1));
  TuplePtr tuple = Tuple::Construct(0, entries);

  TransactionPtr txn = test_db->Begin();
  RelationPtr rel = Relation::Create(txn, 100);
  rel->TableInsert(99, *tuple);
  rel->TableInsert(101, *tuple);
  rel->TableInsert(100, *tuple);

  Cursor* cursor = rel->kvstore->Open();
  int get = 0;
  Slice key;
  Slice value;
  for (int count = 99; count <= 101; ++count) {
    ASSERT_EQ(cursor->Get(key, value, MDB_NEXT), true);
    ASSERT_EQ(*(u64*) key.data(), count);
    ++get;
  }
  ASSERT_EQ(get, 3);
  ASSERT_EQ(cursor->Get(key, value, MDB_NEXT), false);

  rel->kvstore->Close(cursor);
}

TEST(relation, append) {
  std::vector<TupleMeta> entries;
  u8 v1 = std::numeric_limits<u8>::max();
  entries.emplace_back(CHAROID, (char*) &v1, sizeof(v1));
  TuplePtr tuple = Tuple::Construct(0, entries);

  TransactionPtr txn = test_db->Begin();
  RelationPtr rel = Relation::Create(txn, 889);
  for (int i = 1; i <= 10001; ++i) {
    ASSERT_EQ(rel->TableAppend(*tuple), i);
  }

  Slice key;
  Slice value;
  Cursor* cursor = rel->kvstore->Open();
  int get = 0;
  for (int i = 1; cursor->Get(key, value, MDB_NEXT); ++i) {
    ASSERT_EQ(i, *(u64*) key.data());
    ++get;
  }

  ASSERT_EQ(get, 10001);
  txn->Commit();
}

int main(int argc, char* argv[]) {
  system(("rm -rf " + test_db_dir).c_str());
  mkdir(test_db_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

  test_db = Database::Open(test_db_dir.c_str());

  testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();

  Database::Close(test_db);
  system(("rm -rf " + test_db_dir).c_str());
  return ret;
}
