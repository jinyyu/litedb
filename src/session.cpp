#include <thread>
#include <litesql/session.h>

namespace db {

Session::Session(int fd, const char* peerAddr, u16 peerPort) : fd(fd), port(peerPort) {
  strcpy(peer, peerAddr);
  fprintf(stderr, "new connection %s:%d\n", peer, port);
}

Session::~Session() {
  fprintf(stderr, "session closed %s:%d\n", peer, port);
  close(fd);
}

void Session::start() {
  this->sessionCloseCallback();
}

}
