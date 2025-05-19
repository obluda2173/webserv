#include "test_UploadHandler_utils.h"
#include "Connection.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>

std::string getFileContents(const std::string& filename) {
    std::ifstream file(filename);
    if (!file)
        throw std::ios_base::failure("Error opening file");

    std::string contents((std::istreambuf_iterator< char >(file)), std::istreambuf_iterator< char >());

    return contents;
}

Connection* setupConnWithContentLength(std::string filename, size_t contentLength) {
    Connection* conn = new Connection({}, -1, 0, NULL, NULL);
    conn->_request.method = "POST";
    conn->_request.uri = PREFIX + filename;
    conn->_request.version = "HTTP/1.1";
    conn->_request.headers["content-length"] = std::to_string(contentLength);
    conn->setState(Connection::Handling);
    return conn;
}

Connection* setupConnWithoutContentLength(std::string filename) {
    Connection* conn = new Connection({}, -1, 0, NULL, NULL);
    conn->_request.method = "POST";
    conn->_request.uri = PREFIX + filename;
    conn->_request.version = "HTTP/1.1";
    return conn;
}

void removeFile(std::string filepath) {
    std::remove(filepath.data());
    ASSERT_FALSE(std::filesystem::exists(filepath));
}
