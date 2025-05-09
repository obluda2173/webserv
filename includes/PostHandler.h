#ifndef POSTHANDLER_H
#define POSTHANDLER_H

#include <sstream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <filesystem>
#include <algorithm> // for std::search
#include <iterator>  // for std::distance
#include <cstdio> // for std::remove

#include "Router.h"
#include "Connection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

typedef struct inBoundary {
	std::string fileName;			// this is the filename value, file name we need
	std::string noFileNamePath;		// this is the path without the file name (root + uri)
	std::string filePath;			// this is the full path of the file
	std::string fileType;			// this is the file type we need for response
// the path under is root + uri
	bool pathValid = true;			// if something wrong with the path, it will be false
	bool pathExist = false;			// if the dir is exist, it will be true
	bool pathAccessable = false;	// if the dir is accessalbe (we can create file inside it), it will be true

	bool replaceFile = false;		// if the path (root + uri) point to a file, which means we need to replace it, it will be true

	bool fileExist = false;			// if the file exist, it will be true
	bool fileValid = false;			// if the file is a file not a dir, it will be true
	bool fileAccessalbe = false;	// if we can write into the file, it will be true

	bool uploadFailure = false;		// if any error happen, it will be true

	std::vector<char> bodyContent;
} inBoundary;

class PostHandler : public IHandler {
	private:
	  std::string _path;
	  struct stat _pathStat;
	  std::vector<char> _bodyBuf;	// THIS WILL BE CHANGED IN THE FUTURE
	  std::string _boundaryValue;
	  std::vector<std::vector<char>> _parts;
	  std::vector<inBoundary> _subBody;
	  bool _postValidation(Connection* conn, HttpRequest& request, RouteConfig& config);
	  
	  bool _divideBody(const std::vector<char>& bodyBuf, const std::string& boundary);
	  bool _putIntoStruct(void);
	  
	  void _setPath(std::string root, std::string uri);
	  void _setPathWithFileName(void);
	  
	  void _pathValidator(void);
	  void _pathWithFileNameValidator(void);
	  
	  void replaceFile(int index);
	  void _writeIntoFile();
	  
	  void _setResponse(HttpResponse& resp, int statusCode, const std::string& statusMessage, const std::string& contentType, size_t contentLength, IBodyProvider* bodyProvider);
	  void _setErrorResponse(HttpResponse& resp, int code, const std::string& message, const RouteConfig& config);

	  void _generateRespone(Connection* conn, HttpResponse& resp);
	  std::string _generateBody(void);

	  std::string _getMimeType(const std::string& path);
	  
	public:
	  static std::map<std::string, std::string> mimeTypes;
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

/*✅ Step-by-Step: Parse a multipart/form-data Upload
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