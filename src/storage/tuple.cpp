#include <litedb/storage/tuple.h>
#include <assert.h>
#include <stdexcept>
#include <litedb/utils/exception.h>

namespace db {

TuplePtr Tuple::Construct(const std::vector<Slice>& entries) {
  if (entries.empty()) {
    THROW_EXCEPTION("invalid entries");
  }
  u32 headerLen = sizeof(TupleHeaderData) + entries.size() * sizeof(TupleDataMeta);
  u32 totalLen = headerLen;
  for (const Slice& entry: entries) {
    totalLen += entry.size();
  }

  TupleHeaderData* header = (TupleHeaderData*) malloc(totalLen);
  header->headerSize = headerLen;
  u32 dataOffset = 0;

  for (size_t i = 0; i < entries.size(); ++i) {
    const Slice& entry = entries[i];

    header->meta[i].offset = dataOffset;
    header->meta[i].size = entry.size();
    memcpy(((char*) header) + headerLen + dataOffset, entry.data(), entry.size());

    dataOffset += entry.size();
  }

  TuplePtr tuple(new Tuple((char*) header, totalLen));
  tuple->copied_ = true;
  return tuple;
}

void Tuple::Get(int index, Slice& entry) const {
  TupleHeaderData* header = (TupleHeaderData*) tuple_;
  u32 headerLen = header->headerSize;

  if (headerLen < sizeof(TupleHeaderData) + (index + 1) * sizeof(TupleDataMeta)) {
    THROW_EXCEPTION("index out of range");
  }

  u32 offset = header->meta[index].offset;

  size_t entrySize = header->meta[index].size;
  const char* entryData = entrySize > 0 ? (char*) header + headerLen + offset : nullptr;
  entry = Slice(entryData, entrySize);
}

}

