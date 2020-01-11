#include <litedb/bin/initdb.h>
#include<unistd.h>
#include <litedb/utils/env.h>
#include <litedb/utils/elog.h>
#include <litedb/catalog/catalog.h>
#include <sys/stat.h>

namespace db {

#define LOG_INFO(format, ...) do { fprintf(stderr, "INFO [%s:%d] " format "\n", strrchr(__FILE__, '/') + 1, __LINE__, ##__VA_ARGS__); } while(0)
#define LOG_ERROR(format, ...) do { fprintf(stderr, "ERROR [%s:%d] " format "\n", strrchr(__FILE__, '/') + 1, __LINE__, ##__VA_ARGS__); exit(EXIT_FAILURE); } while(0)


void InitCatalog()
{

  if (mkdir(CATALOG_DATABASE, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
    LOG_ERROR("mkdir error %s", strerror(errno));
  }

  CatalogDB = Database::Open("catalog");

  Transaction* txn = CatalogDB->Begin();

  Table* tbl = txn->Open("sys_class");
  const char* test_key = "test_key";
  Entry key(test_key, strlen(test_key));

  const char* test_data = "test_data";
  Entry data(test_data, strlen(test_data));

  tbl->Put(&key, &data);
  fprintf(stderr, "put ok\n");

  Entry out;

  tbl->Get(&key, &out);


  std::string str((char*)out.data, out.size);

  fprintf(stderr, "get  ok %s\n", str.c_str());

  txn->Commit();

  delete(txn);

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
