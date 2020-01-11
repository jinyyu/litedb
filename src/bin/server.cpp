#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <litedb/int.h>
#include <litedb/bin/server.h>
#include <litedb/bin/session.h>
#include <litedb/parser/keywords.h>
#include <mutex>
#include <thread>
#include <signal.h>
#include <memory>

namespace db {

Server::Server(int port)
    : port_(port),
      running_(true) {
  serverFd_ = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
  if (serverFd_ < 0) {
    fprintf(stderr, "create socket error %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  int optval = 1;
  setsockopt(serverFd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  InitKeyword();
}

Server::~Server() {
  close(serverFd_);
}


void Server::Stop() {
  close(serverFd_);
}

void Server::Loop() {
  struct sockaddr_in in_addr;
  memset(&in_addr, 0, sizeof(in_addr));
  in_addr.sin_family = AF_INET;
  in_addr.sin_port = htobe16(port_);

  if (bind(serverFd_, (const sockaddr*) &in_addr, sizeof(in_addr)) < 0) {
    fprintf(stderr, "bind socket error %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (::listen(serverFd_, SOMAXCONN) < 0) {
    fprintf(stderr, "listen socket error %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  fprintf(stdout, "listening on IPv4 address 0.0.0.0:%d\n", port_);

  while (running_) {
    struct sockaddr_in peer;
    socklen_t len = sizeof(in_addr);
    memset(&peer, 0, len);

    int clientFd = accept4(serverFd_, (struct sockaddr*) &peer, &len, SOCK_CLOEXEC);
    if (clientFd < 0) {
      if (errno == EBADF) {
        //stop
        break;
      }
      fprintf(stderr, "accept4 error %s\n", strerror(errno));
      return;
    }
    OnNewConnection(clientFd, &peer);
  }
}

void Server::OnNewConnection(int fd, struct sockaddr_in* addr) {
  char ip[INET_ADDRSTRLEN];
  inet_ntop(addr->sin_family, &addr->sin_addr, ip, INET_ADDRSTRLEN);
  Session* session = new Session(fd, ip, be16toh(addr->sin_port));

  std::lock_guard<std::mutex> guard(this->sessionLock_);
  sessions_[fd] = session;

  session->sessionCloseCallback = [fd, this]() {
    std::lock_guard<std::mutex> guard(this->sessionLock_);

    Session* self = this->sessions_.at(fd);
    delete (self);
    sessions_.erase(fd);
  };

  std::thread thread([session]() {
    session->Loop();
  });
  thread.detach();
}

typedef std::shared_ptr<Server> ServerPtr;

static ServerPtr server = nullptr;

void HandleStop(int) {
  if (server) {
    server->Stop();
  }
}

int Server::Main(const char* workspace, int port) {

  ServerPtr s(new Server((db::u16) port));
  server = s;

  signal(SIGINT, HandleStop);

  server->Loop();
  server = nullptr;
  fprintf(stderr, "server stopped\n");
  return 0;
}

}
