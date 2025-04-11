#ifndef SOCKET_H
#define SOCKET_H

#include <poll.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <stdexcept>
#include <netinet/in.h>

#include <Logger.h>

class Server {
  private:
    struct sockaddr_in _address;
    socklen_t _addrlen;
    int _port;
    int _serverfd;
    Logger *_logger;
    std::vector<struct pollfd> _pfds;
    int _init(int port);
    int _bind();

  public:
    static const int BUFFER_SIZE = 1024;
    Server(Logger *logger);
    ~Server();
    void listen(int port);
    int handleConnections();
    int processConn(int client_socket);
};

#endif // SOCKET_H
