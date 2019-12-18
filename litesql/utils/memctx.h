#ifndef LITESQL_LITESQL_MEMCTX_H_
#define LITESQL_LITESQL_MEMCTX_H_
#include <litesql/int.h>
#include <unordered_set>
#include <list>

namespace db {

struct MemoryContext;
struct Object {

  explicit Object(MemoryContext* ctx);
  virtual ~Object();
  virtual void Drop();

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
extern thread_local MemoryContext* MessageContext;
extern thread_local MemoryContext* TopTransactionContext;
extern thread_local MemoryContext* CurTransactionContext;

void* Malloc(size_t size);
void* Malloc0(size_t size);
void Free(void* ptr);
char* Strdup(const char* str);

}

#endif //LITESQL_LITESQL_MEMCTX_H_
