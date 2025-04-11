#ifndef SOCKET_H
#define SOCKET_H

#include <Logger.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>

class Server {
  private:
    struct sockaddr_in _address;
    socklen_t _addrlen;
    int _port;
    int _serverfd;
    Logger *_logger;
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
