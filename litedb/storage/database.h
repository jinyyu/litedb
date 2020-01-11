#ifndef LITEDB_STORAGE_DATABASE_H_
#define LITEDB_STORAGE_DATABASE_H_
#include <stdexcept>

namespace db {

class Exception : std::runtime_error {
 public:
  explicit Exception(int code, const char* msg)
      : runtime_error(msg),
        code(code) {

  }

  int code;
};

class Transaction;
class Database {
 public:

  //Opens the database
  static Database* Open(const char* path);

  //Closes the database
  static void Close(Database* db);

  virtual ~Database() = default;

  //Begins a transaction
  virtual Transaction* Begin() = 0;

};

class Transaction {
 public:

};

}

#endif //LITEDB_STORAGE_DATABASE_H_
