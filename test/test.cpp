#include <litedb/storage/tuple.h>

using namespace db;

int main(int argc, char* argv[]) {
  Entry entry1("abc", 3);
  Entry entry2("ijks", 4);

  std::vector<Entry> entries;
  entries.push_back(entry1);
  entries.push_back(entry2);

  TuplePtr tuple = Tuple::Construct(entries);

  Entry entry;
  tuple->Get(1, entry);
  std::string str(entry.data, entry.size);
  fprintf(stderr, "---------------%s----\n", str.c_str());


}
