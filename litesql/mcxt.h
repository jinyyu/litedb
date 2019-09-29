#ifndef LITESQL_LITESQL_MCXT_H_
#define LITESQL_LITESQL_MCXT_H_
#include <litesql/int.h>
#include <unordered_set>
#include <list>

namespace db {

struct MemoryContext;
struct Object {

  explicit Object();
  virtual ~Object();

  MemoryContext* ctx;
};

struct MemoryContext {

  ~MemoryContext() { Reset(); }
  static void Init();
  static void Release();
  static MemoryContext* Create(MemoryContext* parent, const char* name);
  static MemoryContext* SwitchTo(MemoryContext* ctx);

  void Reset();

  MemoryContext* parent;
  const char* name;
  std::unordered_set<Object*> objects;
  std::unordered_set<void*> chucks;
  std::list<MemoryContext*> children;

private:
  explicit MemoryContext(const char* name, MemoryContext* parent)
      : parent(parent),
        name(name) {
  }
};

extern thread_local MemoryContext* TopMemoryContext;
extern thread_local MemoryContext* ErrorContext;
extern thread_local MemoryContext* CurTransactionContext;

void* Malloc(size_t size);
void* Malloc0(size_t size);
void Free(void* ptr);
char* Strdup(const char* str);

}

#endif //LITESQL_LITESQL_MCXT_H_
