#include <thread>
#include <litesql/session.h>
#include <litesql/mcxt.h>
#include <litesql/pq.h>

namespace db {

Session::Session(int fd, const char* peerAddr, u16 peerPort) : fd(fd), port(peerPort) {
  strcpy(peer, peerAddr);
  fprintf(stderr, "new connection %s:%d\n", peer, port);
}

Session::~Session() {
  fprintf(stderr, "session closed %s:%d\n", peer, port);
  close(fd);
}

void Session::Start() {

  MemoryContext::Init();

  ProcessStartupPacket();

  MemoryContext::Release();

  this->sessionCloseCallback();
}

void Session::ProcessStartupPacket() {
  StartupPacket packet;
  int ret = PQ_GetBytes(fd, (u8*) &packet, sizeof(packet));
  if (ret == EOF) {
    fprintf(stderr, "incomplete startup packet\n");
    return;
  }
  u32 len = ntohl(packet.length);
  if (len < sizeof(packet) || len > MAX_STARTUP_PACKET_LENGTH) {
    fprintf(stderr, "invalid length of startup packet\n");
    return;
  }
  u32 paramLen = len - sizeof(packet);
  if (paramLen > 0) {
    char* data = (char*) Malloc(paramLen);
    char* ptr = data;
    ret = PQ_GetBytes(fd, (u8*) ptr, paramLen);

    if (ret == EOF) {
      fprintf(stderr, "incomplete startup packet\n");
      return;
    }

    while (paramLen > 0) {
      if (*ptr == '\0') {
        break;
      }

      size_t keyLen = strlen(ptr) + 1;
      if (keyLen > paramLen) {
        fprintf(stderr, "invalid startup packet\n");
        return;
      }
      char* key = ptr;
      ptr += keyLen;
      paramLen -= keyLen;

      size_t valueLen = strlen(ptr) + 1;
      if (valueLen > paramLen) {
        fprintf(stderr, "invalid startup packet\n");
        return;
      }
      char* value = ptr;
      ptr += valueLen;
      paramLen -= valueLen;

      startupParameters[key] = value;
    }

    Free(data);
  }

  for (auto it : startupParameters) {
    fprintf(stderr, "%s:%s\n", it.first.c_str(), it.second.c_str());
  }

}

}
