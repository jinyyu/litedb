#include <litedb/storage/Database.h>

using namespace db;

int main(int argc, char* argv[]) {

  Database* env = Database::Create();
  env->SetMapSize(1024000);
  env->Open("./example.mdb");

  Database::Terminate(env);

}
