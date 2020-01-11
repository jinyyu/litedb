#ifndef LITEDB_SRC_STORAGE_EXCEPTION_H_
#define LITEDB_SRC_STORAGE_EXCEPTION_H_
#include <lmdb.h>
#include <string>
#include <string.h>

namespace db {

#define CHECK_LMDB_ERROR(rc) do { \
    if (rc == MDB_SUCCESS) break; \
    char* msg = mdb_strerror(rc); \
    fprintf(stderr, "ERROR [%s:%d] %s\n", strrchr(__FILE__, '/') + 1, __LINE__, msg); \
    throw BtException(rc, msg); \
} while(0)

}

#endif //LITEDB_SRC_STORAGE_EXCEPTION_H_
