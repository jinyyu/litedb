#ifndef LITEDB_BIN_SERVER_H_
#define LITEDB_BIN_SERVER_H_
#include <mutex>
#include <unordered_map>
#include <litedb/bin/session.h>

namespace db {

class Server {
 public:
  static int Main(const char* workspace, int port);

  void Stop();

  ~Server();

 private:

  explicit Server(int port);

  void Loop();

  void OnNewConnection(int fd, struct sockaddr_in* addr);

  u16 port_;                 // listen port
  int serverFd_;             // server socket
  volatile bool running_;    // is server running?
  std::mutex sessionLock_;   // lock of sessions
  std::unordered_map<int, Session*> sessions_; //sessions
};

}

#endif //LITEDB_BIN_SERVER_H_
