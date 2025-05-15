#ifndef ISENDER_H
#define ISENDER_H

#include <cstddef>

class ISender {
  public:
    virtual ~ISender() {}
    virtual size_t _send(int fd, char* buf, size_t size) = 0;
};



#endif // ISENDER_H
