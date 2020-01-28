#ifndef LITEDB_STORAGE_DATABASE_H_
#define LITEDB_STORAGE_DATABASE_H_
#include <litedb/utils/exception.h>
#include <string>
#include <memory>
#include <litedb/int.h>
#include <litedb/utils/slice.h>

struct MDB_val;

namespace db {

#define DB_DEFAULT_FLAG (0)
#define DB_DEFAULT_OP (0)

class Table;
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

class Transaction {
 public:

  virtual ~Transaction() = default;

  //Opens or create a Table
  virtual Table* Open(const std::string& name, u32 flags) = 0;

  //Commit all the operations of a transaction into the database
  virtual void Commit() = 0;

  //Abandon all the operations of the transaction instead of saving them
  virtual void Abort() = 0;

};

typedef int (TableKeyCompareFunc)(MDB_val* a, MDB_val* b);

class Table {
 public:

  virtual ~Table() = default;

  //Store items into the table
  virtual bool Put(const Slice& key, const Slice& value, u32 flags) = 0;

  //Get items from the table
  virtual bool Get(const Slice& key, Slice& value) = 0;

  //Delete items from the table
  virtual bool Del(const Slice& key, Slice* value = nullptr) = 0;

  //Create a cursor
  virtual Cursor* Open() = 0;

  virtual void Close(Cursor* cursor) = 0;

  //Set a custom key comparison function for a database
  virtual void SetCompare(TableKeyCompareFunc* cmp) = 0;
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
