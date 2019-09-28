#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <glib.h>
#include <unordered_map>
#include <litesql/int.h>
#include <litesql/session.h>
#include <mutex>
#include <thread>

namespace db {

struct Server {
  explicit Server(int port)
      : port(port),
        running(true) {
    serverFd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
    if (serverFd < 0) {
      fprintf(stderr, "create socket error %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    int optval = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  }

  ~Server() {
    close(serverFd);
  }

  void Loop() {
    struct sockaddr_in in_addr;
    memset(&in_addr, 0, sizeof(in_addr));
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(port);

    if (bind(serverFd, (const sockaddr*) &in_addr, sizeof(in_addr)) < 0) {
      fprintf(stderr, "bind socket error %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    if (::listen(serverFd, SOMAXCONN) < 0) {
      fprintf(stderr, "listen socket error %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    while (running) {
      struct sockaddr_in peer;
      socklen_t len = sizeof(in_addr);
      memset(&peer, 0, len);

      int clientFd = accept4(serverFd, (struct sockaddr*) &peer, &len, SOCK_CLOEXEC);
      if (clientFd < 0) {
        fprintf(stderr, "accept4 error %s\n", strerror(errno));
        continue;
      }
      OnNewConnection(clientFd, &peer);
    }
  }

  void OnNewConnection(int fd, struct sockaddr_in* addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(addr->sin_family, &addr->sin_addr, ip, INET_ADDRSTRLEN);
    Session* session = new Session(fd, ip, ntohs(addr->sin_port));

    std::lock_guard<std::mutex> guard(this->sessionLock);
    sessions[fd] = session;

    session->sessionCloseCallback = [fd, this]() {
      std::lock_guard<std::mutex> guard(this->sessionLock);

      Session* self = this->sessions.at(fd);
      delete (self);
      sessions.erase(fd);
    };

    std::thread thread([session]() {
      session->start();
    });
    thread.detach();
  }

  u16 port;                 // listen port
  int serverFd;             // server socket
  volatile bool running;    // is server running?
  std::mutex sessionLock;   // lock of sessions
  std::unordered_map<int, Session*> sessions; //sessions
};

} // db

int main(int argc, char* argv[]) {
  int port;
  GOptionEntry entries[] = {
      {"port", 'p', 0, G_OPTION_ARG_INT, &port, "listen port", NULL},
      {NULL}
  };

  GError* error = NULL;
  GOptionContext* context = g_option_context_new("usage");
  g_option_context_add_main_entries(context, entries, NULL);
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    fprintf(stderr, "option parsing failed: %s\n", error->message);
    exit(EXIT_FAILURE);
  }

  if (port == 0) {
    char* help = g_option_context_get_help(context, true, NULL);
    fprintf(stderr, help);
    free(help);
    exit(EXIT_FAILURE);
  }

  db::Server server((db::u16) port);
  server.Loop();
}