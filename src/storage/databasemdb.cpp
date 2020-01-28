#include <litedb/storage/database.h>
#include "litedb/storage/databasemdb.h"
#include <assert.h>
#include <lmdb.h>

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

bool TableMdb::Put(const Slice& key, const Slice& value, u32 flags) {
  ::MDB_val key_val = {.mv_size = key.size(), .mv_data = (char*) key.data(),};
  ::MDB_val value_val = {.mv_size = value.size(), .mv_data = (char*) value.data(),};

  int rc = mdb_put(trans_->txn_, dbi_, &key_val, &value_val, flags);
  if (rc != MDB_SUCCESS && rc != MDB_KEYEXIST) {
    CHECK_LMDB_ERROR(rc);
  }
  return (rc == MDB_SUCCESS);
}

bool TableMdb::Get(const Slice& key, Slice& value) {
  ::MDB_val key_val = {.mv_size = key.size(), .mv_data = (char*) key.data(),};
  ::MDB_val value_val;

  int rc = mdb_get(trans_->txn_, dbi_, &key_val, &value_val);
  if (rc != MDB_SUCCESS && rc != MDB_KEYEXIST) {
    CHECK_LMDB_ERROR(rc);
  }
  if (rc == MDB_SUCCESS) {
    value = Slice((char*) value_val.mv_data, value_val.mv_size);
  }
  return (rc == MDB_SUCCESS);
}

bool TableMdb::Del(const Slice& key, Slice* value) {
  ::MDB_val key_val = {.mv_size = key.size(), .mv_data = (char*) key.data(),};
  ::MDB_val value_val;
  ::MDB_val* value_ptr;
  if (value) {
    value_val.mv_data = (void*) value->data();
    value_val.mv_size = value->size();
    value_ptr = &value_val;
  } else {
    value_ptr = nullptr;
  }
  int rc = mdb_del(trans_->txn_, dbi_, &key_val, value_ptr);
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

void TableMdb::SetCompare(TableKeyCompareFunc* cmp) {
  if (!set_compare_) {
    return;
  }
  int rc = mdb_set_compare(trans_->txn_, dbi_, (MDB_cmp_func*) cmp);
  if (rc) {
    CHECK_LMDB_ERROR(rc);
  }
  set_compare_ = true;
}

CursorMdb::~CursorMdb() {
  mdb_cursor_close(cursor_);
}

bool CursorMdb::Get(const Slice& key, Slice& value, u32 op) {
  ::MDB_val key_val = {.mv_size = key.size(), .mv_data = (char*) key.data(),};
  ::MDB_val value_val = {.mv_size = value.size(), .mv_data = (char*) value.data(),};

  int rc = mdb_cursor_get(cursor_, &key_val, &value_val, static_cast<MDB_cursor_op>(op));
  if (rc != MDB_SUCCESS && rc != MDB_NOTFOUND) {
    CHECK_LMDB_ERROR(rc);
  }
  if (rc == MDB_SUCCESS) {
    value = Slice((char*) value_val.mv_data, value_val.mv_size);
  }
  return rc == MDB_SUCCESS;
}

bool CursorMdb::Put(const Slice& key, const Slice& value, u32 flags) {
  ::MDB_val key_val = {.mv_size = key.size(), .mv_data = (char*) key.data(),};
  ::MDB_val value_val = {.mv_size = value.size(), .mv_data = (char*) value.data(),};

  int rc = mdb_cursor_put(cursor_, &key_val, &value_val, flags);
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

