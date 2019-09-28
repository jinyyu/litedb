#include <litesql/pq.h>
#include <litesql/int.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <vector>

namespace db {

#define PQ_RECV_BUFFER_SIZE 8192
thread_local static char PqRecvBuffer[PQ_RECV_BUFFER_SIZE];
thread_local static int PqRecvPointer = 0;
thread_local static int PqRecvLength = 0;

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

}

