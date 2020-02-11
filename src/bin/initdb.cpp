#include <litedb/bin/initdb.h>
#include<unistd.h>
#include <litedb/utils/env.h>
#include <litedb/catalog/catalog.h>
#include <litedb/catalog/sys_class.h>
#include <litedb/catalog/sys_attribute.h>
#include <litedb/catalog/sys_index.h>
#include <litedb/storage/relation.h>
#include <sys/stat.h>

namespace db {

#define LOG_INFO(format, ...) do { fprintf(stderr, "INFO [%s:%d] " format "\n", strrchr(__FILE__, '/') + 1, __LINE__, ##__VA_ARGS__); } while(0)
#define LOG_ERROR(format, ...) do { fprintf(stderr, "ERROR [%s:%d] " format "\n", strrchr(__FILE__, '/') + 1, __LINE__, ##__VA_ARGS__); exit(EXIT_FAILURE); } while(0)

static void init_sys_class(TransactionPtr txn);
static void init_sys_attribute(TransactionPtr txn);
static void init_sys_index(TransactionPtr txn);

static void init_sys_class(TransactionPtr txn) {
  SysClass::CreateEntry(txn, SysClassRelationId, SysClassRelationName, true, RELKIND_RELATION, Natts_sys_class);

  SysAttribute::CreateEntry(txn, SysClassRelationId, INT8OID, "relid", Anum_sys_class_id - 1);
  SysAttribute::CreateEntry(txn, SysClassRelationId, NAMEOID, "relname", Anum_sys_class_relname - 1);
  SysAttribute::CreateEntry(txn, SysClassRelationId, BOOLOID, "relhasindex", Anum_sys_class_relhasindex - 1);
  SysAttribute::CreateEntry(txn, SysClassRelationId, CHAROID, "relkind", Anum_sys_class_relkind - 1);
  SysAttribute::CreateEntry(txn, SysClassRelationId, INT2OID, "relnatts", Anum_sys_class_relnatts - 1);

}

static void init_sys_attribute(TransactionPtr txn) {
  SysClass::CreateEntry(txn,
                        SysAttributeRelationId,
                        SysAttributeRelationName,
                        true,
                        RELKIND_RELATION,
                        Natts_sys_attribute);

  SysAttribute::CreateEntry(txn, SysAttributeRelationId, INT8OID, "attrelid", Anum_sys_attribute_attrelid - 1);
  SysAttribute::CreateEntry(txn, SysAttributeRelationId, INT4OID, "atttypid", Anum_sys_attribute_atttypid - 1);
  SysAttribute::CreateEntry(txn, SysAttributeRelationId, NAMEOID, "attname", Anum_sys_attribute_attname - 1);
  SysAttribute::CreateEntry(txn, SysAttributeRelationId, INT2OID, "attnum", Anum_sys_attribute_attnum - 1);
}

void init_sys_index(TransactionPtr txn) {
  SysClass::CreateEntry(txn, SysIndexRelationId, SysIndexRelationName, true, RELKIND_RELATION, Natts_sys_index);

  SysAttribute::CreateEntry(txn, SysIndexRelationId, INT8OID, "indexrelid", Anum_sys_index_indexrelid - 1);
  SysAttribute::CreateEntry(txn, SysIndexRelationId, INT8OID, "indrelid", Anum_sys_index_indrelid - 1);
  SysAttribute::CreateEntry(txn, SysIndexRelationId, INT2OID, "indnatts", Anum_sys_index_indnatts - 1);
  SysAttribute::CreateEntry(txn, SysIndexRelationId, INT2OID, "indnkeyatts", Anum_sys_index_indnkeyatts - 1);
  SysAttribute::CreateEntry(txn, SysIndexRelationId, BOOLOID, "indisunique", Anum_sys_index_indisunique - 1);
  SysAttribute::CreateEntry(txn, SysIndexRelationId, BOOLOID, "indisprimary", Anum_sys_index_indisprimary - 1);
  SysAttribute::CreateEntry(txn, SysIndexRelationId, INT2VECTOROID, "indkey", Anum_sys_index_indkey - 1);
}

void InitCatalog() {

  if (mkdir(CATALOG_DATABASE, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
    LOG_ERROR("mkdir error %s", strerror(errno));
  }

  CatalogDB = Database::Open("catalog");

  TransactionPtr txn = CatalogDB->Begin();

  Relation::Create(txn, SysClassRelationId);
  Relation::Create(txn, SysAttributeRelationId);
  Relation::Create(txn, SysIndexRelationId);

  init_sys_class(txn);
  init_sys_attribute(txn);
  init_sys_index(txn);

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
