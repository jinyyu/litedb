#include <litedb/catalog/catalog.h>

namespace db {

Database* CatalogDB = nullptr;
thread_local TransactionPtr CurrentTransaction = nullptr;

}

