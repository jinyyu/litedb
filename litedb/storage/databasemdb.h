#ifndef LITEDB_STORAGE_DATABASEMDB_H_
#define LITEDB_STORAGE_DATABASEMDB_H_
#include <lmdb.h>
#include <litedb/storage/database.h>
#include <litedb/int.h>
#include "litedb/storage/exception.h"

namespace db {

class DatabaseMdb : public Database {
 public:

  explicit DatabaseMdb();

  ~DatabaseMdb() final;

  Transaction* Begin() final;

  //Open the environment handle
  void Open(const char* path, unsigned int flags, mdb_mode_t mode);

  //Set the size of the memory map to use for this environment
  void SetMapSize(size_t size) {
    int rc = mdb_env_set_mapsize(env_, size);
    CHECK_LMDB_ERROR(rc);
  }

  //Set the maximum number of named databases for the environment
  void DetMaxDBs(size_t size) {
    int rc = mdb_env_set_maxdbs(env_, size);
    CHECK_LMDB_ERROR(rc);
  }

  MDB_env* env_;
};

class TransactionMdb : public Transaction {

};

}

#endif //LITEDB_STORAGE_DATABASEMDB_H_
