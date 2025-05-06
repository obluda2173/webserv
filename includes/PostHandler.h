#ifndef POSTHANDLER_H
#define POSTHANDLER_H

#include <sstream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <filesystem>

#include "Router.h"
#include "Connection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

class PostHandler : public IHandler {
	private:
	  std::string _path;
	  struct stat _pathStat;
	  std::vector<char> _bodyBuf;	// THIS WILL BE CHANGED IN THE FUTURE
	  std::string _boundaryValue;
	  static std::map<std::string, std::string> mimeTypes;
	  bool _getValidation(Connection* conn, HttpRequest& request, RouteConfig& config);
	  void _setErrorResponse(HttpResponse& resp, int code, const std::string& message);
	  void _setGoodResponse(HttpResponse& resp, std::string mimeType, int statusCode, size_t fileSize, IBodyProvider* bodyProvider);
	  std::string _normalizePath(const std::string& root, const std::string& uri);
	  std::string _getMimeType(const std::string& path);
	  std::string _getDirectoryListing(const std::string& path, const std::string& uri);
  
	public:
	  PostHandler();
	  ~PostHandler();
	  void handle(Connection* conn, HttpRequest& req, RouteConfig& config);
  };

#endif

/*Correct behavior: your server should reject the request with a 400 Bad Request status.
Without the boundary, you have no reliable way to parse the multipart body.
Real-world web frameworks (e.g., Apache, nginx, Express.js, Flask) will reject such requests automatically.*/

// Always sanitize the path to avoid directory traversal (../).
// Add checks so clients can’t write arbitrary data into sensitive places.

/*
✅ Step-by-Step: Parse a multipart/form-data Upload
1. Get the boundary from the Content-Type header
Look in the main HTTP headers for something like this:

pgsql
复制
编辑
Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryXYZ123
Extract the value of the boundary, e.g.:

cpp
复制
编辑
std::string boundary = "----WebKitFormBoundaryXYZ123";
2. Split the HTTP body using the boundary
Each part in the multipart body is separated by:

diff
复制
编辑
--<boundary>
and ends with:

diff
复制
编辑
--<boundary>--
So you break the body into parts by splitting on:

css
复制
编辑
--boundary
3. Process each part
Each part looks like this:

pgsql
复制
编辑
Content-Disposition: form-data; name="myfile"; filename="example.txt"
Content-Type: text/plain

<file content here>
You need to:

Find the Content-Disposition header.

Extract:

name="..." (field name)

filename="..." (actual file name)

Then find the blank line (\r\n\r\n) that separates headers from the content.

The data after that is the actual file content.

4. Save the file content
After parsing out the filename, open a file to write the content:

cpp
复制
编辑
std::ofstream out("uploads/" + sanitized_filename, std::ios::binary);
out.write(file_content.data(), file_content.size());
Make sure to:

Sanitize the filename (no .., no /)

Handle binary data correctly (don’t treat it as a string)
*/