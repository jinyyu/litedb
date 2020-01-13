#ifndef LITEDB_CATALOG_CATALOG_H_
#define LITEDB_CATALOG_CATALOG_H_
#include <litedb/storage/database.h>

namespace db {

extern Database* CatalogDB;
extern Database* PublicDB;

#define CATALOG_DATABASE "catalog"
#define PUBLIC_DATABASE "public"

#define NAMEDATALEN 64

}

#endif //LITEDB_CATALOG_CATALOG_H_
