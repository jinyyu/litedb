#include <litesql/pq.h>
#include <litesql/int.h>
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
thread_local static int PqRecvPointer = 0;
thread_local static int PqRecvLength = 0;
thread_local static std::vector<u8> PqSendBuffer;

int PQ_GetBytes(int socket, u8* buf, size_t len) {
  int left = PqRecvLength - PqRecvPointer;
  if (left > 0) {
    memcpy(buf, PqRecvBuffer + PqRecvPointer, left);
    buf += left;
    len -= left;
    PqRecvPointer += left;
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

void PQ_BeginMessage(char msgType) {
  PqSendBuffer.resize(5);
  PqSendBuffer[0] = msgType;
}

void PQ_SendU8(u8 i) {
  PqSendBuffer.push_back(i);
}

void PQ_SendU32(u32 i) {
  i = htonl(i);
  u8* p = (u8*) &i;
  PqSendBuffer.insert(PqSendBuffer.end(), p, p + 4);
}

void PQ_SendString(const char* str) {
  PqSendBuffer.insert(PqSendBuffer.end(), str, str + strlen(str) + 1);
}

void PQ_EndMessage() {
  u32 size = PqSendBuffer.size() - 1;
  size = htonl(size);
  memcpy(PqSendBuffer.data() + 1, &size, 4);
}

int PQ_Flush(int fd) {
  const u8* data = PqSendBuffer.data();
  size_t len = PqSendBuffer.size();

  while (len) {
    ssize_t bytes = send(fd, data, len, 0);
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

    data += bytes;
    len -= bytes;
  }
  return 0;
}

}

