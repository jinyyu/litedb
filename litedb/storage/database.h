#ifndef LITEDB_STORAGE_DATABASE_H_
#define LITEDB_STORAGE_DATABASE_H_
#include <stdexcept>
#include <string>

namespace db {

class Exception : std::runtime_error {
 public:
  explicit Exception(int code, const char* msg)
      : runtime_error(msg),
        code(code) {

  }

  int code;
};

class Table;
class Transaction;

class Database {
 public:

  //Opens the database
  static Database* Open(const char* path);

  //Closes the database
  static void Close(Database* db);

  virtual ~Database() = default;

  //Create a transaction
  virtual Transaction* Begin() = 0;

};

class Transaction {
 public:

  virtual ~Transaction() = default;

  //Opens or create a table
  virtual Table* Open(const std::string& name) = 0;

  //Commit all the operations of a transaction into the database
  virtual void Commit() = 0;

  //Abandon all the operations of the transaction instead of saving them
  virtual void Abort() = 0;

};

struct Entry {
  explicit Entry() = default;
  explicit Entry(const void* data, size_t size) : size(size), data((void*) data) {}
  explicit Entry(const char* data, size_t size) : size(size), data((void*) data) {}
  size_t size;    /**< size of the data item */
  void* data;    /**< address of the data Entry */
};

class Table {
 public:

  virtual ~Table() = default;

  virtual void Put(Entry* key, Entry* value) = 0;

  virtual void Get(Entry* key, Entry* value) = 0;

  virtual void Del(Entry* key, Entry* value) = 0;

};

}

#endif //LITEDB_STORAGE_DATABASE_H_
