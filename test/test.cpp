#include <map>

int main(int argc, char* argv[]) {
  std::map<int,int> maps;
  maps[5] = 5;
  auto it = maps.find(5);
  it->second = 2000;
  fprintf(stderr, "%d", maps[5]);


}
