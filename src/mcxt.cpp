#include <litesql/mcxt.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

namespace db {

thread_local MemoryContext* TopMemoryContext = nullptr;
thread_local MemoryContext* ErrorContext = nullptr;
thread_local MemoryContext* CurTransactionContext = nullptr;

Object::Object() {
  assert(CurTransactionContext);

  ctx = CurTransactionContext;
  ctx->objects.insert(this);
}

Object::~Object() {
  ctx->objects.erase(this);
}

void MemoryContext::Init() {
  assert(!TopMemoryContext);
  TopMemoryContext = new MemoryContext("TopMemoryContext", nullptr);
  CurTransactionContext = TopMemoryContext;
  ErrorContext = Create(TopMemoryContext, "ErrorContext");
}

void MemoryContext::Release() {
  assert(TopMemoryContext);
  delete (TopMemoryContext);
  TopMemoryContext = nullptr;
  CurTransactionContext = nullptr;
  ErrorContext = nullptr;
}

MemoryContext* MemoryContext::Create(MemoryContext* parent, const char* name) {
  assert(parent);
  MemoryContext* ctx = new MemoryContext(name, parent);
  parent->children.push_back(ctx);
  return ctx;
}

MemoryContext* MemoryContext::SwitchTo(MemoryContext* ctx) {
  MemoryContext* old = CurTransactionContext;
  CurTransactionContext = ctx;
  return old;
}

void MemoryContext::Reset() {
  for (auto obj : objects) {
    delete (obj);
  }
  objects.clear();

  for (auto c : chucks) {
    free(c);
  }
  chucks.clear();

  for (auto ch : children) {
    delete (ch);
  }
  children.clear();
}

struct Chucks {
  MemoryContext* ctx;
  u8 data[0];
};

void* Malloc(size_t size) {
  assert(CurTransactionContext);

  Chucks* c = (Chucks*) malloc(sizeof(Chucks) + size);
  c->ctx = CurTransactionContext;
  CurTransactionContext->chucks.insert(c);
  return (void*) c->data;
}

void* Malloc0(size_t size) {
  void* ret = Malloc(size);
  memset(ret, 0, size);
  return ret;
}

void Free(void* ptr) {
  Chucks* c = (Chucks*) ((u8*) ptr - sizeof(MemoryContext*));
  c->ctx->chucks.erase(c);
  free(c);
}

char* Strdup(const char* str) {
  if (!str) {
    return nullptr;
  }
  size_t len = strlen(str);
  if (len == 0) {
    return (char*) "";
  }
  char* ret = (char*) Malloc(len + 1);
  memcpy(ret, str, len + 1);
  return ret;
}

}
