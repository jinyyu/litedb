#ifndef LITESQL_WORKER_H
#define LITESQL_WORKER_H
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <litesql/int.h>
#include <functional>

namespace db {

struct Session {
  explicit Session(int fd, const char* peerAddr, u16 peerPort);
  ~Session();

  void Start();
  void ProcessStartupPacket();

  int fd;                       // the docker
  u16 port;                     // peer port
  char peer[INET_ADDRSTRLEN];   //peer ip
  std::function<void()> sessionCloseCallback;
};

} // db

#endif //LITESQL_WORKER_H
