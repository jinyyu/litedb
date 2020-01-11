#ifndef LITEDB_UTILS_ENV_H_
#define LITEDB_UTILS_ENV_H_
#include <unordered_set>
#include <memory>
#include <string.h>

namespace db {

class Environment;

class Object {
 public:
  explicit Object();

  virtual ~Object() = default;
};

class Environment {
 public:
  Environment();

  ~Environment();

  void* Malloc(size_t size) {
    void* mem = malloc(size);
    memories_.insert(mem);
    return mem;
  }

  void* Malloc0(size_t size) {
    void* mem = malloc(size);
    memset(mem, 0, size);
    memories_.insert(mem);
    return mem;
  }

  char* Strdup(const char* str) {
    size_t size = strlen(str) + 1;
    char* dup = (char*) Malloc(size);
    memcpy(dup, str, size);
    return dup;
  }

  void Free(void* memory);

  void Drop(Object* obj);

  void ReleaseMemory();

 private:
  friend class Object;
  std::unordered_set<void*> memories_;
  std::unordered_set<Object*> objects_;
};

typedef std::shared_ptr<Environment> EnvironmentPtr;

extern thread_local EnvironmentPtr SessionEnv;

}

#endif //LITEDB_UTILS_ENV_H_
