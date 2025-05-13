#include "Connection.h"

static std::string ROOT = "tests/unittests/test_files/UploadHandler";
static std::string PREFIX = "/uploads/";
std::string getFileContents(const std::string& filename);
Connection* setupConnWithContentLength(std::string filename, size_t contentLength);
Connection* setupConnWithoutContentLength(std::string filename);
void removeFile(std::string filepath);
