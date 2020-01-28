#include <litedb/storage/database.h>
#include <gtest/gtest.h>

using namespace db;
static Database* test_db;
static std::string test_db_dir = "test_database_dir";

TEST(database, commit) {
  TransactionPtr txn = test_db->Begin();
  Table* tbl = txn->Open("test1", MDB_CREATE);
  Slice key("test key");

  Slice value( "test_data");
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
