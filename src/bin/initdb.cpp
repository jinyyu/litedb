#include <litedb/bin/initdb.h>
#include<unistd.h>
#include <litedb/utils/env.h>
#include <litedb/catalog/catalog.h>
#include <sys/stat.h>
#include <lmdb.h>

namespace db {

#define LOG_INFO(format, ...) do { fprintf(stderr, "INFO [%s:%d] " format "\n", strrchr(__FILE__, '/') + 1, __LINE__, ##__VA_ARGS__); } while(0)
#define LOG_ERROR(format, ...) do { fprintf(stderr, "ERROR [%s:%d] " format "\n", strrchr(__FILE__, '/') + 1, __LINE__, ##__VA_ARGS__); exit(EXIT_FAILURE); } while(0)

void InitCatalog() {

  if (mkdir(CATALOG_DATABASE, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
    LOG_ERROR("mkdir error %s", strerror(errno));
  }

  CatalogDB = Database::Open("catalog");

  TransactionPtr txn = CatalogDB->Begin();

  Table* tbl = txn->Open("sys_class", MDB_CREATE);
  const char* test_key = "test_key";
  Entry key(test_key, strlen(test_key));

  const char* test_data = "test_data";
  Entry data(test_data, strlen(test_data));

  tbl->Put(&key, &data, DB_DEFAULT_FLAG);
  fprintf(stderr, "put ok\n");

  Cursor* cursor = tbl->Open();

  key.size = 0;
  key.data = nullptr;
  data.size = 0;
  data.data = nullptr;

  while (cursor->Get(&key, &data, MDB_NEXT)) {
    std::string k((const char*) key.data, key.size);
    std::string v((const char*) data.data, data.size);
    printf("key: '%s', value: '%s'\n", k.c_str(), v.c_str());
  }

  tbl->Close(cursor);

  txn->Commit();

  txn = nullptr;

  Database::Close(CatalogDB);
}

int InitDBMain(const char* workspace) {
  SessionEnv = std::make_shared<Environment>();
  LOG_INFO("create database: %s", workspace);

  if (mkdir(workspace, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
    LOG_ERROR("mkdir error %s", strerror(errno));
  }

  chdir(workspace);

  InitCatalog();

  return 0;
}
}
