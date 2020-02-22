#ifndef LITEDB_CATALOG_CATALOG_H_
#define LITEDB_CATALOG_CATALOG_H_
#include <litedb/storage/database.h>
#include <litedb/storage/tuple.h>

namespace db {

extern Database* CatalogDB;
extern thread_local TransactionPtr CurrentTransaction;

#define CATALOG_DATABASE "catalog"

#define NAMEDATALEN 64

}

#endif //LITEDB_CATALOG_CATALOG_H_
