#include <litedb/storage/database.h>

using namespace db;

int main(int argc, char* argv[]) {

  Database* db = Database::Open("./example.mdb");

  Database::Close(db);

}
