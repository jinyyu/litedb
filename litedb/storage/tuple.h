#ifndef LITEDB_STORAGE_TUPLE_H_
#define LITEDB_STORAGE_TUPLE_H_
#include <litedb/int.h>
#include <vector>
#include <litedb/utils/env.h>
#include <litedb/utils/slice.h>
#include <litedb/storage/database.h>
#include <litedb/catalog/sys_type.h>

namespace db {

#pragma pack(push, 1)
struct TypeMeta {
  u32 type;      //type of the data, little endian
  u32 offset;    //offset of the data, little endian
  u32 size;      //size of data, little endian
};

// the Tuple format
// | headerSize | TupleDataMeta1 | TupleDataMeta2| .... | TupleDataMetaN | TupleData1 | TupleData2 | .... | TupleDataN |
struct TupleHeaderData {
  u32 headerSize;
  TypeMeta meta[0];
};
#pragma pack(pop)

struct TupleMeta {
  explicit TupleMeta(u32 type, const char* data, u32 size)
      : type(type),
        data(data),
        size(size) {}

  TupleMeta() = default;

  explicit TupleMeta(u16* u) : type(INT2OID), data((const char*) u), size(sizeof(u)) {}
  explicit TupleMeta(u32* u) : type(INT4OID), data((const char*) u), size(sizeof(u)) {}
  explicit TupleMeta(u64* u) : type(INT8OID), data((const char*) u), size(sizeof(u)) {}

  u32 type;   //type of the data;
  const char* data; //address of the data
  u32 size;   //size of the data
};

class Tuple;
typedef std::shared_ptr<Tuple> TuplePtr;

class Tuple {
 public:
  explicit Tuple(char* tuple, u32 len)
      : id_(0),
        tuple_(tuple),
        len_(len),
        copied_(false) {
  }

  static TuplePtr Construct(const std::vector<TupleMeta>& entries);

  ~Tuple() {
    if (copied_) {
      free(tuple_);
    }
  }

  u32 columns() const;

  u32 GetType(int index) const {
    TupleMeta entry;
    Get(index, entry);
    return entry.type;
  }

  void Get(int index, TupleMeta& entry) const;

  template<typename T>
  T GetBasicType(int index) const {
    TupleMeta entry;
    Get(index, entry);
    if (sizeof(T) != entry.size) {
      THROW_EXCEPTION("invalid tuple")
    }
    return *(T*) entry.data;
  }

  Slice GetSlice(int index) const {
    TupleMeta entry;
    Get(index, entry);
    return Slice(entry.data, entry.size);
  }

  void GetTupleData(Slice& slice) const {
    slice.assign(tuple_, len_);
  }

  bool ContainsID() const { return id_ > 0; }
  void SetID(u64 id) { id_ = id; }
  u64 GetID() const { return id_; }

 private:
  u64 id_;
  char* tuple_;  //tuple data
  u32 len_;      //tuple data size
  bool copied_;  //is tuple copied?
};

}

#endif //LITEDB_STORAGE_TUPLE_H_
