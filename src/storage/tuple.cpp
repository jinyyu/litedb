#include <litedb/storage/tuple.h>
#include <assert.h>
#include <stdexcept>
#include <litedb/utils/exception.h>

namespace db {

#define MAX_COLUMNS (128)

TuplePtr Tuple::Construct(const std::vector<TupleMeta>& entries) {
  if (entries.empty()) {
    THROW_EXCEPTION("invalid entries");
  }
  u32 headerLen = sizeof(TupleHeaderData) + entries.size() * sizeof(TypeMeta);
  u32 totalLen = headerLen;
  for (const TupleMeta& meta: entries) {
    totalLen += meta.size;
  }

  TupleHeaderData* header = (TupleHeaderData*) malloc(totalLen);
  header->headerSize = headerLen;
  u32 dataOffset = 0;

  for (size_t i = 0; i < entries.size(); ++i) {
    const TupleMeta& entry = entries[i];

    header->meta[i].type = entry.type;
    header->meta[i].offset = dataOffset;
    header->meta[i].size = entry.size;
    memcpy(((char*) header) + headerLen + dataOffset, entry.data, entry.size);

    dataOffset += entry.size;
  }

  TuplePtr tuple(new Tuple((char*) header, totalLen));
  tuple->copied_ = true;
  return tuple;
}

u32 Tuple::columns() const {
  TupleHeaderData* header = (TupleHeaderData*) tuple_;
  size_t n = (header->headerSize - sizeof(TupleHeaderData)) / sizeof(TypeMeta);
  assert(n > 0);
  return n;
}

void Tuple::Get(int index, TupleMeta& entry) const {
  TupleHeaderData* header = (TupleHeaderData*) tuple_;

  if (header->headerSize < sizeof(TupleHeaderData) + (index + 1) * sizeof(TypeMeta)) {
    THROW_EXCEPTION("index out of range");
  }

  u32 offset = header->meta[index].offset;

  size_t entrySize = header->meta[index].size;
  entry.type = header->meta[index].type;
  entry.size = entrySize;
  entry.data = entrySize > 0 ? (char*) header + header->headerSize + offset : nullptr;
}

}

