#ifndef BUFFER_H
#define BUFFER_H

#include "ISender.h"
#include "ResponseWriter.h"
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>

class Buffer {
  private:
    std::vector< char > _content;
    size_t _size;

  public:
    Buffer();

    void print();
    void write(IResponseWriter* wrtr);
    ssize_t recv(int fd);

    ssize_t send(ISender* sender, int fd);
    void advance(size_t count);
    void clear();
    size_t size() const;
    void assign(std::string s);
    char* data();
};

#endif // BUFFER_H
