#include <litedb/storage/Database.h>
#include "DatabaseMdb.h"

namespace db {

#define MDB_DEFAULT_MODE 0644 /* -rw-r--r-- */
#define MAX_MDB_TABLES 128

Database* Database::Create() {
  DatabaseMdb* mdb = new DatabaseMdb();
  return mdb;
}

void Database::Terminate(Database* env) {
  delete (env);
}

DatabaseMdb::DatabaseMdb() : env_(nullptr) {
  int rc = mdb_env_create(&env_);
  CHECK_LMDB_ERROR(rc);

  rc = mdb_env_set_maxdbs(env_, MAX_MDB_TABLES);
  CHECK_LMDB_ERROR(rc);
}

DatabaseMdb::~DatabaseMdb() {
  mdb_env_close(env_);
}

void DatabaseMdb::Open(const char* path) {
  int rc = mdb_env_open(env_, path, 0, MDB_DEFAULT_MODE);
  CHECK_LMDB_ERROR(rc);
}

}

