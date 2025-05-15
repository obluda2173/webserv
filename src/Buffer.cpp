#include "Buffer.h"

Buffer::Buffer() {
    _content.resize(1024);
    _size = 0;
}

void Buffer::write(IResponseWriter* wrtr) { _size += wrtr->write(_content.data() + _size, _content.size() - _size); }

void Buffer::recv(int fd) { _size += ::recv(fd, _content.data() + _size, _content.size() - _size, 0); }

void Buffer::send(ISender* sender, int fd) { advance(sender->_send(fd, _content.data(), _size)); }

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
