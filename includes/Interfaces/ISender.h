#ifndef ISENDER_H
#define ISENDER_H

#include <cstddef>
#include <sys/types.h>

class ISender {
  public:
    virtual ~ISender() {}
    virtual ssize_t _send(int fd, char* buf, size_t size) = 0;
};

#endif // ISENDER_H
