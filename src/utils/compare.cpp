#include <litedb/utils/compare.h>
#include <assert.h>
#include <litedb/catalog/sys_type.h>
#include <litedb/storage/tuple.h>
#include <litedb/utils/elog.h>

namespace db {

template<typename T>
int BasicTypeCmp(T* t1, size_t size1, T* t2, size_t size2) {
  assert(size1 == sizeof(T) && size2 == sizeof(T));
  T va = *(T*) t1;
  T vb = *(T*) t2;
  if (va < vb) {
    return -1;
  } else if (va > vb) {
    return 1;
  } else {
    return 0;
  }
}

TypeCmpCallback* GetCmpFunction(u32 type) {
  TypeCmpCallback* ret;
  switch (type) {
    case CHAROID:
    case BOOLOID: {
      ret = i8_cmp;
      break;
    }
    case INT2OID: {
      ret = i16_cmp;
      break;
    }
    case INT4OID: {
      ret = i32_cmp;
      break;
    }
    case INT8OID: {
      ret = i64_cmp;
      break;
    }
    case NAMEOID: {
      ret = name_cmp;
      break;
    }
    default: {
      assert(false);
      ret = nullptr;
    }
  }
  return ret;
}

int i8_cmp(Entry* a, Entry* b) {
  return BasicTypeCmp<i8>((i8*) a->data, a->size, (i8*) b->data, b->size);
}

int i16_cmp(Entry* a, Entry* b) {
  return BasicTypeCmp<i16>((i16*) a->data, a->size, (i16*) b->data, b->size);
}

int i32_cmp(Entry* a, Entry* b) {
  return BasicTypeCmp<i32>((i32*) a->data, a->size, (i32*) b->data, b->size);
}

int i64_cmp(Entry* a, Entry* b) {
  return BasicTypeCmp<i64>((i64*) a->data, a->size, (i64*) b->data, b->size);
}

int u64_cmp(Entry* a, Entry* b) {
  return BasicTypeCmp<u64>((u64*) a->data, a->size, (u64*) b->data, b->size);
}

int name_cmp(Entry* a, Entry* b) {
  assert(a->size == NAMEDATALEN && b->size == NAMEDATALEN);
  return strncmp((char*) a->data, (char*) b->data, NAMEDATALEN);
}

int index_cmp(Entry* a, Entry* b) {
  Tuple tuple1((char*) a->data, a->size);
  u32 column1 = tuple1.columns();

  Tuple tuple2((char*) b->data, a->size);
  u32 column2 = tuple2.columns();

  u32 minColumn = std::min(column1, column2);

  for (u32 i = 0; i < minColumn; ++i) {
    TupleMeta meta1;
    TupleMeta meta2;
    Entry entry1;
    Entry entry2;

    tuple1.Get(i, meta1);
    tuple2.Get(i, meta2);
    assert(meta1.type == meta2.type);

    entry1.size = meta1.size;
    entry1.data = (void*) meta1.data;
    entry2.size = meta2.size;
    entry2.data = (void*) meta2.data;

    TypeCmpCallback* cb = GetCmpFunction(meta1.type);
    assert(cb);

    int ret = cb(&entry1, &entry2);
    if (ret != 0) {
      return ret;
    }
  }

  if (column1 == column2) {
    return 0;
  } else if (column1 < column2) {
    return -1;
  } else {
    return 1;
  }
}

}



