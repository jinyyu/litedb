#include <litedb/storage/database.h>
#include "litedb/storage/databasemdb.h"

namespace db {

#define MDB_DEFAULT_MODE 0644 /* -rw-r--r-- */
#define MAX_MDB_TABLES 128
#define DEFAULT_MEMORY_SIZE (1024 * 1024 * 1024)

Database* Database::Open(const char* path) {
  DatabaseMdb* mdb = new DatabaseMdb();

  try {
    mdb->SetMapSize(DEFAULT_MEMORY_SIZE);
    mdb->DetMaxDBs(MAX_MDB_TABLES);

    mdb->Open(path, 0, MDB_DEFAULT_MODE);

  } catch (Exception& exp) {
    delete (mdb);
    throw exp;
  }
  return mdb;
}

void Database::Close(Database* env) {
  delete (env);
}

DatabaseMdb::DatabaseMdb() : env_(nullptr) {
  mdb_env_create(&env_);
}

DatabaseMdb::~DatabaseMdb() {
  mdb_env_close(env_);
}

Transaction* DatabaseMdb::Begin() {
  return nullptr;
}

void DatabaseMdb::Open(const char* path, unsigned int flags, mdb_mode_t mode) {
  int rc = mdb_env_open(env_, path, 0, MDB_DEFAULT_MODE);
  CHECK_LMDB_ERROR(rc);
}

}

