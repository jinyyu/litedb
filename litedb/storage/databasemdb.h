#ifndef LITEDB_STORAGE_DATABASEMDB_H_
#define LITEDB_STORAGE_DATABASEMDB_H_
#include <lmdb.h>
#include <litedb/storage/database.h>
#include <litedb/int.h>
#include <memory>
#include <list>
#include <unordered_map>
#include <litedb/utils/exception.h>

namespace db {

class DatabaseMdb : public Database {
 public:

  explicit DatabaseMdb();

  ~DatabaseMdb() final;

  TransactionPtr Begin() final;

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

  //Opens thd table
  KVStore* Open(const std::string& name, u32 flags) final;

  void Commit() final;

  void Abort() final;

  DatabaseMdb* mdb_;
  MDB_txn* txn_;
  std::unordered_map<std::string, KVStore*> tables_;
};

class KVStoreMdb : public KVStore {
 public:
  explicit KVStoreMdb(TransactionMdb* trans, MDB_dbi dbi)
      : set_compare_(false),
        trans_(trans),
        dbi_(dbi) {

  }

  virtual ~KVStoreMdb() final;

  bool Put(const Slice& key, const Slice& value, u32 flags) final;

  bool Get(const Slice& key, Slice& value) final;

  bool Del(const Slice& key, Slice* value = nullptr) final;

  Cursor* Open() final;

  void Close(Cursor* cursor) final;

  void SetCompare(TypeCmpCallback cmp) final;

  bool SetCompare() const {
    return set_compare_;
  }

  bool set_compare_;
  TransactionMdb* trans_;
  MDB_dbi dbi_;
  std::list<Cursor*> cursors_;
};

class CursorMdb : public Cursor {
 public:
  explicit CursorMdb(MDB_cursor* cursor) : cursor_(cursor) {}

  ~CursorMdb() final;

  bool Get(Slice& key, Slice& value, u32 op) final;

  bool Put(const Slice& key, const Slice& value, u32 flags) final;

  void Del(u32 flags) final;

  MDB_cursor* cursor_;
};

}

#endif //LITEDB_STORAGE_DATABASEMDB_H_
