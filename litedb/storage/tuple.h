#ifndef LITEDB_STORAGE_TUPLE_H_
#define LITEDB_STORAGE_TUPLE_H_
#include <litedb/int.h>
#include <vector>
#include <litedb/utils/env.h>
#include <litedb/utils/slice.h>
#include <litedb/storage/database.h>

namespace db {

#pragma pack(push, 1)
struct TupleDataMeta {
  u32 offset;    //offset of the data, little endian
  u32 size;      //size of data, little endian
};

// the Tuple format
// | headerSize | TupleDataMeta1 | TupleDataMeta2| .... | TupleDataMetaN | TupleData1 | TupleData2 | .... | TupleDataN |
struct TupleHeaderData {
  u32 headerSize;
  TupleDataMeta meta[0];
};
#pragma pack(pop)

class Tuple;
typedef std::shared_ptr<Tuple> TuplePtr;

class Tuple {
 public:
  explicit Tuple(char* tuple, u32 len)
      : tuple_(tuple),
        len_(len),
        copied_(false) {
  }

  static TuplePtr Construct(const std::vector<Entry>& entries);

  ~Tuple() {
    if (copied_) {
      free(tuple_);
    }
  }

  void Get(int index, Entry& entry) const;

  template<typename T>
  T GetInt(int index) const {
    Entry entry;
    Get(index, entry);
    if (sizeof(T) != entry.size) {
      THROW_EXCEPTION("invalid tuple")
    }
    return *(T*) entry.data;
  }

  Slice GetSlice(int index) const {
    Entry entry;
    Get(index, entry);
    return Slice(entry.data, entry.size);
  }

 private:
  char* tuple_;  //tuple data
  u32 len_;      //tuple data size
  bool copied_;  //is tuple copied?
};

}

#endif //LITEDB_STORAGE_TUPLE_H_
