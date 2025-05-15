#ifndef BUFFER_H
#define BUFFER_H

#include "ISender.h"
#include "ResponseWriter.h"
#include <string>
#include <sys/socket.h>
#include <vector>

class Buffer {
  private:
    std::vector< char > _content;
    size_t _size;

  public:
    Buffer();

    void write(IResponseWriter* wrtr);
    void recv(int fd);
    void send(ISender* sender, int fd);
    void advance(size_t count);
    void clear();
    size_t size() const;
    void assign(std::string s);
    char* data();
};

#endif // BUFFER_H
