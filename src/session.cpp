#include <thread>
#include <litesql/session.h>
#include <litesql/mcxt.h>
#include <litesql/pq.h>
#include <litesql/elog.h>

namespace db {

Session::Session(int fd, const char* peerAddr, u16 peerPort)
    : fd(fd),
      port(peerPort) {
  strcpy(peer, peerAddr);
  fprintf(stderr, "new connection %s:%d\n", peer, port);
}

Session::~Session() {
  fprintf(stderr, "session closed %s:%d\n", peer, port);
  close(fd);
}

void Session::Start() {

  MemoryContext::Init();
  int status = ProcessStartupPacket();
  if (status != STATUS_OK) {
    MemoryContext::Release();
    this->sessionCloseCallback();
    return;
  }

}

int Session::ProcessStartupPacket() {
  StartupPacket packet;
  int ret = PQ_GetBytes(fd, (u8*) &packet, sizeof(packet));
  if (ret == EOF) {
    eReport(COMMERROR, "incomplete startup packet");
    return STATUS_ERROR;
  }
  u32 len = ntohl(packet.length);
  if (len < sizeof(packet) || len > MAX_STARTUP_PACKET_LENGTH) {
    eReport(COMMERROR, "invalid length of startup packet");
    return STATUS_ERROR;
  }
  u32 paramLen = len - sizeof(packet);
  if (paramLen > 0) {
    char* data = (char*) Malloc(paramLen);
    char* ptr = data;
    ret = PQ_GetBytes(fd, (u8*) ptr, paramLen);

    if (ret == EOF) {
      eReport(COMMERROR, "incomplete startup packet");
      return STATUS_ERROR;
    }

    while (paramLen > 0) {
      if (*ptr == '\0') {
        break;
      }

      size_t keyLen = strlen(ptr) + 1;
      if (keyLen > paramLen) {
        eReport(COMMERROR, "invalid startup packet");
        return STATUS_ERROR;
      }
      char* key = ptr;
      ptr += keyLen;
      paramLen -= keyLen;

      size_t valueLen = strlen(ptr) + 1;
      if (valueLen > paramLen) {
        eReport(COMMERROR, "invalid startup packet");
        return STATUS_ERROR;
      }
      char* value = ptr;
      ptr += valueLen;
      paramLen -= valueLen;

      eReport(DEBUG, "%s:%s", key, value);

    }

    Free(data);
  }
  return STATUS_OK;
}

}
