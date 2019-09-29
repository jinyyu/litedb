#ifndef LITESQL_WORKER_H
#define LITESQL_WORKER_H
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <litesql/int.h>
#include <functional>
#include <atomic>

namespace db {

struct Session {
  explicit Session(int fd, const char* peerAddr, u16 peerPort);
  ~Session();

  void Start();

  /*
   * Read a client's startup packet and do something according to it.
   *
   * Returns STATUS_OK or STATUS_ERROR
   */
  int ProcessStartupPacket();

  bool forceClose;
  int fd;                       // the docker
  u16 port;                     // peer port
  char peer[INET_ADDRSTRLEN];   //peer ip
  std::function<void()> sessionCloseCallback;
};


extern thread_local Session* CurSession;

} // db

#endif //LITESQL_WORKER_H
