#ifndef LITEDB_STORAGE_EXCEPTION_H_
#define LITEDB_STORAGE_EXCEPTION_H_
#include <lmdb.h>
#include <string>
#include <string.h>
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

#define CHECK_LMDB_ERROR(rc) do { \
    if (rc == MDB_SUCCESS) break; \
    char* msg = mdb_strerror(rc); \
    fprintf(stderr, "ERROR [%s:%d] %s\n", strrchr(__FILE__, '/') + 1, __LINE__, msg); \
    throw Exception(rc, msg); \
} while(0)

}

#endif //LITEDB_STORAGE_EXCEPTION_H_
