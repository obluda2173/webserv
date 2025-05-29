#include "Buffer.h"
#include <sys/types.h>

Buffer::Buffer() {
    _content.resize(8192);
    _size = 0;
}

void Buffer::write(IResponseWriter* wrtr) { _size += wrtr->write(_content.data() + _size, _content.size() - _size); }

ssize_t Buffer::recv(int fd) {
    ssize_t r = ::recv(fd, _content.data() + _size, _content.size(), 0);
    if (r <= 0)
        return r;
    _size += r;
    return r;
}

ssize_t Buffer::send(ISender* sender, int fd) {
    ssize_t r = sender->_send(fd, _content.data(), _size);
    if (r <= 0)
        return r;
    advance(r);
    return r;
}

void Buffer::advance(size_t count) {
    memmove(_content.data(), _content.data() + count, _size - count);
    _size -= count;
}

void Buffer::assign(std::string s) {
    if (s.length() > _content.size()) {
        std::cout << "string is bigger than readBuf" << std::endl;
        return;
    }
    for (size_t i = 0; i < s.length(); i++)
        _content[i] = s[i];
    _size = s.length();
}
char* Buffer::data() { return _content.data(); }

void Buffer::clear() { _size = 0; }

size_t Buffer::size() const { return _size; }

void Buffer::print() { std::cout << std::string(_content.data(), _size) << std::endl; }
