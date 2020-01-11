#ifndef LITEDB_SRC_STORAGE_DATABASEMDB_H_
#define LITEDB_SRC_STORAGE_DATABASEMDB_H_
#include <lmdb.h>
#include <litedb/storage/Database.h>
#include <litedb/int.h>
#include "Exception.h"
namespace db {

class DatabaseMdb : public Database {
 public:
  explicit DatabaseMdb();

  ~DatabaseMdb() final;

  void SetMapSize(size_t size) final {
    int rc = mdb_env_set_mapsize(env_, size);
    CHECK_LMDB_ERROR(rc);
  }

  void Open(const char* path) final;

 private:
  MDB_env* env_;
};

}

#endif //LITEDB_SRC_STORAGE_DATABASEMDB_H_
