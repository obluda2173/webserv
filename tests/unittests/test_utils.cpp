#include "IConnectionHandler.h"
#include "IIONotifier.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <unistd.h>

void reuseSocket(int socketfd) {
    int yes = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        std::cerr << "Failed to reuse socket: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

void setAddr(std::string ip, int port, struct addrinfo& hints, struct addrinfo** res) {
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // Use IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

    // Convert port to a string
    std::string port_str = std::to_string(port);

    int status = getaddrinfo(ip.c_str(), port_str.c_str(), &hints, res);
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        exit(1);
    }
}

void setSvrAddr(sockaddr_in& svrAddr, int port) {
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_port = htons(port);
    ASSERT_GT(inet_pton(AF_INET, "127.0.0.1", &(svrAddr.sin_addr)), 0) << "inet_pton: " << strerror(errno);
}

int countOpenFileDescriptors() {
    int count = 0;
    DIR* dir = opendir("/proc/self/fd");
    if (dir != nullptr) {
        while (readdir(dir) != nullptr) {
            count++;
        }
        closedir(dir);
        // Adjust count to exclude ".", ".." and the dir fd itself
        count -= 3;
    }
    return count;
}

void sendMsgInBatches(std::string msg, int clientfd, int batchSize) {
    // cutting the msg into parts and send
    std::vector< std::string > chunks;
    for (std::size_t i = 0; i < msg.length(); i += batchSize) {
        send(clientfd, msg.substr(i, batchSize).c_str(), msg.substr(i, batchSize).length(), 0);
    }
}

bool allZero(std::vector< std::string > msgs) {
    for (std::vector< std::string >::iterator it = msgs.begin(); it != msgs.end(); it++) {
        if ((*it).length() > 0)
            return false;
    }
    return true;
}

bool checkNotification(IIONotifier* ioNotifier, t_notif wantNotif) {
    std::vector< t_notif > notifs = ioNotifier->wait();
    for (size_t i = 0; i < notifs.size(); i++) {
        if (wantNotif.notif == notifs[i].notif && wantNotif.fd == notifs[i].fd)
            return true;
    }

    return false;
}

void readUntilREADY_TO_WRITE(IIONotifier* _ioNotifier, IConnectionHandler* connHdlr, int fd) {
    while (!checkNotification(_ioNotifier, t_notif{fd, READY_TO_WRITE}))
        connHdlr->handleConnection(fd, READY_TO_READ);
}

void readTillNothingMoreToRead(IIONotifier* _ioNotifier, IConnectionHandler* _connHdlr, int _conn) {
    while (checkNotification(_ioNotifier, t_notif{_conn, READY_TO_READ}))
        _connHdlr->handleConnection(_conn, READY_TO_READ);
}

std::string getResponseConnHdlr(int _conn, IConnectionHandler* _connHdlr, int _clientfd) {
    char buffer[1024];
    ssize_t r;
    _connHdlr->handleConnection(_conn, READY_TO_WRITE);
    r = recv(_clientfd, buffer, 1024, 0);
    if (r == -1) {
        std::cout << "recv: " << std::strerror(errno) << std::endl;
        exit(1);
    }
    buffer[r] = '\0';
    return buffer;
}

std::string getRandomString(size_t length) {
    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string randomString;

    std::srand(static_cast< unsigned int >(std::time(0))); // Seed for random number generator

    for (size_t i = 0; i < length; ++i) {
        randomString += chars[std::rand() % chars.length()];
    }

    return randomString;
}

void expectNotif(t_notif notif, int fd, e_notif eNotif) {
    EXPECT_EQ(notif.fd, fd) << "not the correct filedescriptor";
    EXPECT_EQ(notif.notif, eNotif) << "no notifiactions found";
}

std::string getFileContents(const std::string& filename) {
    std::ifstream file(filename);
    if (!file)
        throw std::ios_base::failure("Error opening file");

    std::string contents((std::istreambuf_iterator< char >(file)), std::istreambuf_iterator< char >());

    return contents;
}

void removeFile(std::string filepath) {
    std::remove(filepath.data());
    ASSERT_FALSE(std::filesystem::exists(filepath));
}

int getRandomNumber(int min, int max) {
    std::random_device rd;                           // Obtain a random number from hardware
    std::mt19937 eng(rd());                          // Seed the generator
    std::uniform_int_distribution<> distr(min, max); // Define the range

    return distr(eng);
}

Connection* setupConnWithTransferEncoding(std::string filename) {
    Connection* conn = new Connection({}, -1, "", NULL, NULL);
    conn->_request.method = "POST";
    conn->_request.uri = PREFIX + filename;
    conn->_request.version = "HTTP/1.1";
    conn->_request.headers["transfer-encoding"] = "chunked";
    conn->setState(Connection::Handling);
    return conn;
}

Connection* setupConnWithContentLength(std::string filename, size_t contentLength) {
    Connection* conn = new Connection({}, -1, "", NULL, NULL);
    conn->_request.method = "POST";
    conn->_request.uri = PREFIX + filename;
    conn->_request.version = "HTTP/1.1";
    conn->_request.headers["content-length"] = std::to_string(contentLength);
    conn->setState(Connection::Handling);
    return conn;
}

Connection* setupConnWithoutContentLength(std::string filename) {
    Connection* conn = new Connection({}, -1, "", NULL, NULL);
    conn->_request.method = "POST";
    conn->_request.uri = PREFIX + filename;
    conn->_request.version = "HTTP/1.1";
    return conn;
}
