#include "Buffer.h"

Buffer::Buffer() {
    _content.resize(1024);
    _size = 0;
}

void Buffer::writeNew(IResponseWriter* wrtr) {
    size_t bytesWritten = wrtr->write(_content.data() + _size, _content.size() - _size);
    _size += bytesWritten;
}

void Buffer::recvNew(int fd) {
    ssize_t r = recv(fd, _content.data() + _size, _content.size() - _size, 0);
    _size += r;
}

void Buffer::send(ISender* sender, int fd) {
    size_t bytesSent = sender->_send(fd, _content.data(), _size);
    if (bytesSent > 0)
        advance(bytesSent);
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
    _content.assign(s.begin(), s.end());
    _size = s.length();
}
char* Buffer::data() { return _content.data(); }

void Buffer::clear() { _size = 0; }

size_t Buffer::size() const { return _size; }
