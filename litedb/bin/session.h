#ifndef LITEDB_SESSION_H
#define LITEDB_SESSION_H
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <litedb/int.h>
#include <functional>
#include <atomic>
#include <vector>

namespace db {

class Session {
 public:
  explicit Session(int fd, const char* peerAddr, u16 peerPort);
  ~Session();

  void Loop();

  /*
   * Read a client's startup packet and do something according to it.
   *
   * Returns STATUS_OK or STATUS_ERROR
   */
  int ProcessStartupPacket();

  /*
   * Client authentication starts here.  If there is an error, this
   * Returns STATUS_OK or STATUS_ERROR
   */
  int ClientAuthentication();

  /*
   * FlushErrorState --- flush the error state after error recovery
   */
  void FlushErrorState();

  /* ----------------
   * ReadyForQuery - tell client that we are ready for a new query
   * ----------------
   */
  void ReadyForQuery();

  void SendCommand(char c, const char* msg, int len);

  void ExecSimpleQuery(char* queryString, size_t queryLen);

  bool forceClose;
  int fd;                       // the docker
  u16 port;                     // peer port
  char peer[INET_ADDRSTRLEN];   //peer ip
  std::vector<u8> sendBuffer;
  std::function<void()> sessionCloseCallback;
  std::string database;
  std::string user;
  std::string client_encoding;

};

extern thread_local Session* CurSession;

} // db

#endif
