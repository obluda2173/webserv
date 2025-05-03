#include "ConnectionHandler.h"
#include "HttpParser.h"
#include "IIONotifier.h"
#include "logging.h"
#include <errno.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

ConnectionHandler::ConnectionHandler(ILogger& l, IIONotifier& ep) : _logger(l), _ioNotifier(ep) {}

ConnectionHandler::~ConnectionHandler(void) {
    for (std::map<int, ConnectionInfo>::iterator it = _connections.begin(); it != _connections.end(); it++) {
        close(it->first);
        delete _parsers[it->first];
    }
}

void ConnectionHandler::_addClientConnection(int conn, struct sockaddr_storage theirAddr) {
    logConnection(_logger, theirAddr);
    ConnectionInfo connInfo;
    connInfo.addr = theirAddr;
    connInfo.fd = conn;
    _connections[conn] = connInfo;
    _parsers[conn] = new HttpParser(_logger);
    _responses[conn] = {"", 0};
    _ioNotifier.add(conn);
}

void ConnectionHandler::_removeClientConnection(int conn) {
    ConnectionInfo connInfo = _connections[conn];
    _connections.erase(connInfo.fd);
    _responses.erase(connInfo.fd); // TODO: secure all the raises agains exceptions being thrown
    delete _parsers[connInfo.fd];
    _parsers.erase(connInfo.fd);
    _ioNotifier.del(connInfo.fd);
    close(connInfo.fd);
    logDisconnect(_logger, connInfo.addr);
}

int ConnectionHandler::_acceptNewConnection(int socketfd) {
    struct sockaddr_storage theirAddr;
    int addrlen = sizeof(theirAddr);
    int conn = accept(socketfd, (struct sockaddr*)&theirAddr, (socklen_t*)&addrlen);
    if (conn < 0) {
        _logger.log("ERROR", "accept: " + std::string(strerror(errno)));
        exit(1);
    }
    _addClientConnection(conn, theirAddr);
    return conn;
}

void ConnectionHandler::_readPipeline(int conn) {
    IHttpParser* prsr = _parsers.at(conn);
    char buffer[1024];
    ssize_t r = recv(conn, buffer, 1024, 0);
    buffer[r] = '\0';
    char* b = buffer;
    while (*b) {
        prsr->feed(b, 1);
        if (prsr->error()) {
            prsr->resetPublic();
            _responses[conn].response += "HTTP/1.1 400 Bad Request\r\n"
                                         "\r\n";

            _responses[conn].statusCode = 400;
            _ioNotifier.modify(conn, READY_TO_WRITE);
            return;
        }
        if (prsr->ready()) {
            prsr->resetPublic();
            _responses[conn].response += "HTTP/1.1 200 OK\r\n"
                                         "Content-Length: 4\r\n"
                                         "\r\n"
                                         "pong";
            _responses[conn].statusCode = 200;
            _ioNotifier.modify(conn, READY_TO_WRITE);
        }
        b++;
    }
    return;
}

void ConnectionHandler::_sendPipeline(int conn) {
    send(conn, _responses[conn].response.c_str(), _responses[conn].response.length(), 0);

    std::string Response4xx = "HTTP/1.1 4";
    if (_responses[conn].statusCode == 400) {
        _removeClientConnection(conn);
    } else {
        _responses[conn].response.clear();
        _ioNotifier.modify(conn, READY_TO_READ);
        delete _parsers[conn];
        _parsers[conn] = new HttpParser(_logger);
    }
}

int ConnectionHandler::handleConnection(int fd, e_notif notif) {
    try {
        switch (notif) {
        case READY_TO_READ:
            _readPipeline(fd);
            break;
        case READY_TO_WRITE:
            _sendPipeline(fd);
            break;
        case CLIENT_HUNG_UP:
            _removeClientConnection(fd);
            break;
        case BROKEN_CONNECTION:
            break;
        }
        return fd;
    } catch (std::out_of_range& e) {
        return _acceptNewConnection(fd);
    }
}
