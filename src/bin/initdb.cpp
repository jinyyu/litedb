#include <litedb/bin/initdb.h>
#include<unistd.h>
#include <litedb/utils/env.h>
#include <litedb/catalog/catalog.h>
#include <litedb/catalog/sys_class.h>
#include <litedb/storage/relation.h>
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

  std::vector<u64> relations;
  std::vector<TuplePtr> tuples;

  SysClass::InitCatalogs(relations, tuples);

  assert(relations.size() == tuples.size());

  TransactionPtr txn = CatalogDB->Begin();

  for (size_t i = 0; i < relations.size(); ++i) {
    u64 relID = relations[i];
    TuplePtr tuple = tuples[i];
    u64 tupleID = tuple->GetInt<u64>(0);

    RelationPtr rel = Relation::OpenTable(txn, relations[i]);
    rel->InsertTuple(tupleID, *tuple);
    LOG_INFO("insert tuple (%lu, %lu)", relID, tupleID);
  }

  txn->Commit();

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
