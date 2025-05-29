#ifndef SYSTEMSENDER_H
#define SYSTEMSENDER_H

#include "ISender.h"
#include <cstddef>
#include <sys/socket.h>

class SystemSender : public ISender {
    virtual ssize_t _send(int fd, char* buf, size_t n) { return send(fd, buf, n, 0); }
};

#endif // SYSTEMSENDER_H
