#ifndef LITEDB_STORAGE_DATABASEMDB_H_
#define LITEDB_STORAGE_DATABASEMDB_H_
#include <lmdb.h>
#include <litedb/storage/database.h>
#include <litedb/int.h>
#include <memory>
#include <unordered_map>
#include "litedb/storage/exception.h"

namespace db {

typedef std::shared_ptr<Transaction> TransactionPtr;

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
 public:
  explicit TransactionMdb(DatabaseMdb* mdb_, MDB_txn* txn)
      : mdb_(mdb_),
        txn_(txn) {

  }

  ~TransactionMdb() final;

  Table* Open(const std::string& name) final;

  void Commit() final;

  void Abort() final;

  DatabaseMdb* mdb_;
  MDB_txn* txn_;
  std::unordered_map<std::string, Table*> tables_;
};

class TableMdb : public Table {
 public:
  explicit TableMdb(TransactionMdb* trans, MDB_dbi dbi)
      : trans_(trans),
        dbi_(dbi) {

  }

  void Put(Entry* key, Entry* value) final;

  void Get(Entry* key, Entry* value) final;

  void Del(Entry* key, Entry* value) final;

  TransactionMdb* trans_;
  MDB_dbi dbi_;
};

}

#endif //LITEDB_STORAGE_DATABASEMDB_H_
