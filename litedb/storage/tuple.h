#ifndef LITEDB_STORAGE_TUPLE_H_
#define LITEDB_STORAGE_TUPLE_H_
#include <litedb/int.h>
#include <vector>
#include <litedb/utils/env.h>
#include <litedb/storage/database.h>

namespace db {

#pragma pack(push, 1)
struct TupleDataMeta {
  u32 offset;    // offset of the data, in big endian
  u32 size;      //size of data, in big endian
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
      : tuple(tuple),
        len(len),
        copied(false) {
  }

  static TuplePtr Construct(const std::vector<Entry>& entries);

  ~Tuple() {
    if (copied) {
      free(tuple);
    }
  }

  void Get(int index, Entry& entry) const;

  char* tuple;  //tuple data
  u32 len;      //tuple data size
  bool copied;  // is tuple copied?
};

}

#endif //LITEDB_STORAGE_TUPLE_H_
