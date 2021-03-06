#ifndef LITEDB_STORAGE_DATABASE_H_
#define LITEDB_STORAGE_DATABASE_H_
#include <litedb/utils/exception.h>
#include <string>
#include <memory>
#include <litedb/int.h>
#include <litedb/utils/slice.h>
#include <litedb/utils/compare.h>

struct MDB_val;

namespace db {

class KVStore;
class Transaction;
typedef std::shared_ptr<Transaction> TransactionPtr;
class Cursor;

class Database {
 public:

  //Opens the database
  static Database* Open(const char* path);

  //Closes the database
  static void Close(Database* db);

  virtual ~Database() = default;

  //Create a transaction
  virtual TransactionPtr Begin() = 0;

};

class Relation;
class Transaction {
 public:

  virtual ~Transaction() = default;

  //Opens or create a KVStore
  virtual KVStore* Open(const std::string& name, u32 flags) = 0;

  //Commit all the operations of a transaction into the database
  virtual void Commit() = 0;

  //Abandon all the operations of the transaction instead of saving them
  virtual void Abort() = 0;

  virtual Relation* GetOpenRelation(i64 relid) = 0;
  virtual void InsertOpenRelation(i64 relid, Relation* rel) = 0;
};

class KVStore {
 public:

  virtual ~KVStore() = default;

  //Store items into the KVStore
  virtual bool Put(const Slice& key, const Slice& value, u32 flags) = 0;

  //Get items from the KVStore
  virtual bool Get(const Slice& key, Slice& value) = 0;

  //Delete items from the KVStore
  virtual bool Del(const Slice& key, Slice* value = nullptr) = 0;

  //Create a cursor
  virtual Cursor* Open() = 0;

  virtual void Close(Cursor* cursor) = 0;

  //Set a custom key comparison function for a database
  virtual void SetCompare(TypeCmpCallback cmp) = 0;
};

class Cursor {
 public:

  virtual ~Cursor() = default;

  //Retrieve by cursor
  virtual bool Get(Slice& key, Slice& value, u32 op) = 0;

  //Store by cursor
  virtual bool Put(const Slice& key, const Slice& value, u32 flags) = 0;

  //Delete current key/data pair
  virtual void Del(u32 flags) = 0;
};

}

#endif //LITEDB_STORAGE_DATABASE_H_
