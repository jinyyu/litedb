#include <litedb/utils/env.h>
#include <assert.h>

namespace db {

Object::Object() {
  assert(SessionEnv);
  SessionEnv->objects_.insert(this);
}

Environment::Environment() {

}

Environment::~Environment() {
  ReleaseMemory();
}

void Environment::Free(void* memory) {
  auto it = memories_.find(memory);
  assert(it != memories_.end());
  memories_.erase(it);
  free(memory);
}

void Environment::Drop(Object* obj) {
  auto it = objects_.find(obj);
  assert(it != objects_.end());
  objects_.erase(it);
  delete (obj);
}

void Environment::ReleaseMemory() {
  for (void* mem : memories_) {
    free(mem);
  }
  memories_.clear();

  for (Object* obj: objects_) {
    delete (obj);
  }

  objects_.clear();
}

thread_local EnvironmentPtr SessionEnv = nullptr;

}

