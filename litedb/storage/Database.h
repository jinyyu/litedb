#ifndef LITEDB_LITEDB_STORAGE_DATABASE_H_
#define LITEDB_LITEDB_STORAGE_DATABASE_H_
#include <stdexcept>

namespace db {

class BtException : std::runtime_error {
 public:
  explicit BtException(int code, const char* msg) : runtime_error(msg), code(code) {}

  int code;
};

class Transation;
class Database {
 public:
  static Database* Create();

  static void Terminate(Database* env);

  virtual ~Database() = default;

  virtual void SetMapSize(size_t size) = 0;

  virtual void Open(const char* path) = 0;

  virtual Transation* Begin() = 0;

};

}

#endif //LITEDB_LITEDB_STORAGE_DATABASE_H_
