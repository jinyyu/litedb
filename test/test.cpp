#include <map>

int main(int argc, char* argv[]) {
  std::map<int,int> maps;
  maps[5] = 5;

  auto it = maps.begin();
  it--;
  it--;
  fprintf(stderr, "%d", it->first);


}
