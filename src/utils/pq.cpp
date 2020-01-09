#include <litedb/utils/pq.h>
#include <litedb/int.h>
#include <litedb/utils/env.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace db {

#define PQ_RECV_BUFFER_SIZE 8192

thread_local static char PqRecvBuffer[PQ_RECV_BUFFER_SIZE];
thread_local static size_t PqRecvPointer = 0;
thread_local static size_t PqRecvLength = 0;

PQMessage* MakePQMessage(u8 type, size_t dataLen) {
  u32 totalLen = dataLen + 5;
  PQMessage* msg = (PQMessage*) SessionEnv->Malloc(totalLen);
  msg->type = type;
  msg->len = htobe32(dataLen + sizeof(u32));
  return msg;
}

u32 PQMessage::PutU8(u32 offset, u8 i) {
  memcpy(data + offset, &i, 1);
  return offset + 1;
}

u32 PQMessage::PutU16(u32 offset, u16 i) {
  i = htobe16(i);
  memcpy(data + offset, &i, 2);
  return offset + 2;
}

u32 PQMessage::PutU32(u32 offset, u32 i) {
  i = htobe32(i);
  memcpy(data + offset, &i, 4);
  return offset + 4;
}

u32 PQMessage::PutU64(u32 offset, u64 i) {
  i = htobe64(i);
  memcpy(data + offset, &i, 8);
  return offset + 8;
}

u32 PQMessage::PutData(u32 offset, u8* dataPtr, u32 dataLen) {
  memcpy(data + offset, dataPtr, dataLen);
  return offset + dataLen;
}

void PQ_Reset() {
  PqRecvPointer = 0;
  PqRecvLength = 0;
}

int PQ_GetBytes(int socket, u8* buf, size_t len) {
  size_t left = PqRecvLength - PqRecvPointer;
  if (left > 0) {
    size_t bytes = std::min(len, left);

    memcpy(buf, PqRecvBuffer + PqRecvPointer, bytes);
    PqRecvPointer += bytes;

    buf += bytes;
    len -= bytes;
  }

  while (len > 0) {
    PqRecvPointer = 0;
    PqRecvLength = 0;

    size_t bytes = read(socket, PqRecvBuffer, PQ_RECV_BUFFER_SIZE);
    if (bytes < 0) {
      if (errno == EINTR) {
        /* Ok if interrupted */
        continue;
      } else {
        return EOF;
      }
    } else if (bytes == 0) {
      return EOF;
    }

    if (bytes == len) {
      memcpy(buf, PqRecvBuffer, len);
      PqRecvPointer += bytes;
      return 0;
    } else if (bytes < len) {
      memcpy(buf, PqRecvBuffer, bytes);
      buf += bytes;
      len -= bytes;
      continue;
    } else {
      // bytes > len
      memcpy(buf, PqRecvBuffer, len);
      PqRecvLength = bytes;
      PqRecvPointer = len;
      return 0;
    }
  }
  return 0;
}

void PQ_Append(std::vector<u8>& buffer, PQMessage* msg) {
  u32 totalLen = be32toh(msg->len) + 1;
  u8* ptr = (u8*) msg;
  buffer.insert(buffer.end(), ptr, ptr + totalLen);
}

int PQ_Flush(int fd, const std::vector<u8>& buffer) {
  const u8* ptr = buffer.data();
  size_t size = buffer.size();
  return PQ_Flush(fd, ptr, size);
}

int PQ_Flush(int fd, const u8* ptr, size_t size) {
  while (size > 0) {
    ssize_t bytes = send(fd, ptr, size, 0);
    if (bytes < 0) {
      if (errno == EINTR) {
        /* Ok if interrupted */
        continue;
      } else {
        return EOF;
      }
    } else if (bytes == 0) {
      return EOF;
    }
    ptr += bytes;
    size -= bytes;
  }
  return 0;
}

}

