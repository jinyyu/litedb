#include <litesql/mcxt.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

namespace db {

thread_local MemoryContext* TopMemoryContext = nullptr;
thread_local MemoryContext* CurTransactionContext = nullptr;

Object::Object() {
  assert(CurTransactionContext);

  ctx = CurTransactionContext;
  ctx->objects.insert(this);
}

Object::~Object() {
  ctx->objects.erase(this);
}

MemoryContext::~MemoryContext() {
  for (auto obj : objects) {
    delete (obj);
  }

  for (auto c : chucks) {
    free(c);
  }

  for (auto ch : children) {
    delete (ch);
  }
}

void MemoryContext::Init() {
  assert(!TopMemoryContext);
  TopMemoryContext = new MemoryContext("top memory context");
  CurTransactionContext = TopMemoryContext;
}

void MemoryContext::Release() {
  if (TopMemoryContext) {
    delete (TopMemoryContext);
    TopMemoryContext = nullptr;
    CurTransactionContext = nullptr;
  }
}

MemoryContext* MemoryContext::Create(MemoryContext* parent, const char* name) {
  assert(parent);
  MemoryContext* ctx = new MemoryContext(name);
  parent->children.push_back(ctx);
  return ctx;
}

MemoryContext* MemoryContext::SwitchTo(MemoryContext* ctx) {
  MemoryContext* old = CurTransactionContext;
  CurTransactionContext = ctx;
  return old;
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

}
