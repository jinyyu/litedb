#include <litedb/storage/tuple.h>
#include <assert.h>
#include <endian.h>
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
  header->headerSize = htobe32(headerLen);
  u32 dataOffset = 0;

  for (size_t i = 0; i < entries.size(); ++i) {
    const Entry& entry = entries[i];

    header->meta[i].offset = htobe32(dataOffset);
    header->meta[i].size = htobe32(entry.size);
    memcpy(((char*) header) + headerLen + dataOffset, entry.data, entry.size);

    dataOffset += entry.size;
  }

  TuplePtr tuple(new Tuple((char*) header, totalLen));
  tuple->copied = true;
  return tuple;
}

void Tuple::Get(int index, Entry& entry) const {
  TupleHeaderData* header = (TupleHeaderData*) tuple;
  u32 headerLen = be32toh(header->headerSize);

  u32 offset = be32toh(header->meta[index].offset);
  entry.size = be32toh(header->meta[index].size);
  entry.data = ((char*) header) + headerLen + offset;
}

}

