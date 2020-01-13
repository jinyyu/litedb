#include <litedb/storage/database.h>
#include "litedb/storage/databasemdb.h"
#include <assert.h>

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

TransactionPtr DatabaseMdb::Begin() {
  MDB_txn* txn = nullptr;
  int rc = mdb_txn_begin(env_, nullptr, 0, &txn);
  CHECK_LMDB_ERROR(rc);
  return TransactionPtr(new TransactionMdb(this, txn));
}

void DatabaseMdb::Open(const char* path, unsigned int flags, mdb_mode_t mode) {
  int rc = mdb_env_open(env_, path, 0, MDB_DEFAULT_MODE);
  CHECK_LMDB_ERROR(rc);
}

TransactionMdb::~TransactionMdb() {
  for (auto it : tables_) {
    delete (it.second);
  }

  if (txn_) {
    Abort();
  }
}

Table* TransactionMdb::Open(const std::string& name, u32 flags) {
  auto it = tables_.find(name);
  if (it != tables_.end()) {
    return it->second;
  }

  MDB_dbi dbi;
  int rc = mdb_dbi_open(txn_, name.c_str(), flags, &dbi);
  CHECK_LMDB_ERROR(rc);
  TableMdb* tbl = new TableMdb(this, dbi);
  tables_[name] = tbl;
  return tbl;
}

void TransactionMdb::Commit() {
  int rc = mdb_txn_commit(txn_);
  CHECK_LMDB_ERROR(rc);
  txn_ = nullptr;
}

void TransactionMdb::Abort() {
  mdb_txn_abort(txn_);
  txn_ = nullptr;
}

TableMdb::~TableMdb() {
  for (Cursor* cursor: cursors_) {
    delete (cursor);
  }
}

bool TableMdb::Put(Entry* key, Entry* value, u32 flags) {
  int rc = mdb_put(trans_->txn_, dbi_, (MDB_val*) key, (MDB_val*) value, flags);
  if (rc != MDB_SUCCESS && rc != MDB_KEYEXIST) {
    CHECK_LMDB_ERROR(rc);
  }
  return (rc == MDB_SUCCESS);
}

bool TableMdb::Get(Entry* key, Entry* value) {
  int rc = mdb_get(trans_->txn_, dbi_, (MDB_val*) key, (MDB_val*) value);
  if (rc != MDB_SUCCESS && rc != MDB_KEYEXIST) {
    CHECK_LMDB_ERROR(rc);
  }
  return (rc == MDB_SUCCESS);
}

bool TableMdb::Del(Entry* key, Entry* value) {
  int rc = mdb_del(trans_->txn_, dbi_, (MDB_val*) key, (MDB_val*) value);
  if (rc != MDB_SUCCESS && rc != MDB_KEYEXIST) {
    CHECK_LMDB_ERROR(rc);
  }
  return (rc == MDB_SUCCESS);
}

Cursor* TableMdb::Open() {
  MDB_cursor* mdb_cursor;
  int rc = mdb_cursor_open(trans_->txn_, dbi_, &mdb_cursor);
  CHECK_LMDB_ERROR(rc);
  Cursor* cursor = new CursorMdb(mdb_cursor);
  cursors_.push_back(cursor);
  return cursor;
}

void TableMdb::Close(Cursor* cursor) {
  for (auto it = cursors_.begin(); it != cursors_.end(); ++it) {
    if (cursor == *it) {
      it = cursors_.erase(it);
      delete (cursor);
      return;
    }
  }
  assert(false);
}


CursorMdb::~CursorMdb() {
  mdb_cursor_close(cursor_);
}

bool CursorMdb::Get(Entry* key, Entry* value, u32 op) {
  int rc = mdb_cursor_get(cursor_, (MDB_val*) key, (MDB_val*) value, static_cast<MDB_cursor_op>(op));
  if (rc != MDB_SUCCESS && rc != MDB_NOTFOUND) {
    CHECK_LMDB_ERROR(rc);
  }
  return rc == MDB_SUCCESS;
}

bool CursorMdb::Put(Entry* key, Entry* value, u32 flags) {
  int rc = mdb_cursor_put(cursor_, (MDB_val*) key, (MDB_val*) value, flags);
  if (rc != MDB_SUCCESS && rc != MDB_NOTFOUND) {
    CHECK_LMDB_ERROR(rc);
  }
  return rc == MDB_SUCCESS;
}

void CursorMdb::Del(u32 flags) {
  int rc = mdb_cursor_del(cursor_, flags);
  CHECK_LMDB_ERROR(rc);
}


}

