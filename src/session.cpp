#include <thread>
#include <litesql/session.h>
#include <litesql/mcxt.h>

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

  this->sessionCloseCallback();
}

void Session::ProcessStartupPacket() {
  u32 len;
  u32 version;

}

}
