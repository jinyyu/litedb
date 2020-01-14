#include <litedb/storage/tuple.h>
#include <assert.h>
#include <stdexcept>
#include <litedb/utils/exception.h>

namespace db {

TuplePtr Tuple::Construct(const std::vector<Entry>& entries) {
  if (entries.empty()) {
    THROW_EXCEPTION("invalid entries");
  }
  u32 headerLen = sizeof(TupleHeaderData) + entries.size() * sizeof(TupleDataMeta);
  u32 totalLen = headerLen;
  for (const Entry& entry: entries) {
    totalLen += entry.size;
  }

  TupleHeaderData* header = (TupleHeaderData*) malloc(totalLen);
  header->headerSize = headerLen;
  u32 dataOffset = 0;

  for (size_t i = 0; i < entries.size(); ++i) {
    const Entry& entry = entries[i];

    header->meta[i].offset = dataOffset;
    header->meta[i].size = entry.size;
    memcpy(((char*) header) + headerLen + dataOffset, entry.data, entry.size);

    dataOffset += entry.size;
  }

  TuplePtr tuple(new Tuple((char*) header, totalLen));
  tuple->copied_ = true;
  return tuple;
}

void Tuple::Get(int index, Entry& entry) const {
  TupleHeaderData* header = (TupleHeaderData*) tuple_;
  u32 headerLen = header->headerSize;

  u32 offset = header->meta[index].offset;
  entry.size = header->meta[index].size;
  entry.data = entry.size > 0 ? (char*) header + headerLen + offset : nullptr;
}

}

