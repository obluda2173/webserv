// #include <HttpResponse.h>
// #include <iostream>
// #include <sstream>

// std::string generate400ErrorWebpage(HttpResponseMessage& _responseMessage, HttpRequest& request)
// {
// 	std::string result;
// 	std::string body;
	
// 	body += "<html>";
// 	body += "<head>";
// 	body += "<title>" + _responseMessage.statusCode + " " + _responseMessage.statusMessage + "</title>";
// 	body += "</head>";
// 	body += "<body>";
// 	body += "<h1>" + _responseMessage.statusCode + " " + _responseMessage.statusMessage + "</h1>";
// 	body += "<p>Request Method: " + request.method + "</p>";
// 	body += "<p>Request URI: " + request.uri + "</p>";
// 	body += "</body>";
// 	body += "</html>";
// 	result += request.version;
// 	result += " " + _responseMessage.statusCode + " " + _responseMessage.statusMessage + "\r\n";
// 	result += "Content-Type: text/html\r\n";
// 	result += "Content-Length: " + std::to_string(body.length()) + "\r\n";
// 	result += "Connection: close\r\n\r\n";
// 	result += body + "\r\n\r\n";
// 	return result;
// }

// std::string parseUri(HttpRequest& request)
// {
// 	std::string uriPath;
// 	if (request.uri.find(request.headers["Host"]) != std::string::npos) { //AVOID THE ABSOLUTE PATH WHICH INCLUDES THE HOST NAME
// 		uriPath = request.uri.substr(request.uri.find(request.headers["Host"]) + request.headers["Host"].length());
// 	} else {
// 		if (request.uri.find("file://") != std::string::npos) { //AVOID THE PATH WHICH INCLUDES THE FILE PROTOCOL
// 			uriPath = request.uri.substr(request.uri.find("file://") + 7);
// 		} else {
// 			uriPath = request.uri;
// 		}
// 	}
// 	return uriPath;
// }

// std::string checkHost(HttpRequest& request, Logger& logger, HttpResponseMessage& _responseMessage)
// {
// 	std::string result;
	
// 	if (request.headers["Host"] != "HOST NAME") { // THIS LINE "HOST NAME" REFER TO THE HOST NAME WE GOT FROM THE CONFIGURATION FILE
// 		_responseMessage.statusCode = "400";
// 		_responseMessage.statusMessage = "Bad Request";
// 		logger.log("ERROR", "checkHost: Bad Request");
// 		result = generate400ErrorWebpage(_responseMessage, request);
// 		return result;
// 	}
// 	return result;
// }

// void checkConnection(HttpRequest& request, Logger& logger, HttpResponseMessage& _responseMessage)
// {	
// 	if (request.headers["Connection"] == "close") {
// 		_responseMessage.isClosed = true;
// 	} else if (request.headers["Connection"] == "keep-alive") {
// 		_responseMessage.isClosed = false;
// 	}
// }

// std::string checkAccept(HttpRequest& request, Logger& logger, HttpResponseMessage& _responseMessage)
// {
// 	std::string result;
// 	bool isValid = false;
// 	std::string value = request.headers["Accept"];
// 	std::istringstream iss(value);
// 	std::string token;
// 	std::vector<std::string> acceptTypes;
// 	while (std::getline(iss, token, ',')) {
// 		token.erase(0, token.find_first_not_of(" "));
// 		acceptTypes.push_back(token);
// 	}
// 	for (size_t i = 0; i < acceptTypes.size(); i++) {
// 		std::string token2 = acceptTypes[i];
// 		size_t semicolon = token2.find(';');
// 		if (semicolon != std::string::npos) {
// 			token2 = token2.substr(0, semicolon);
// 		}
// 		// CHECK THE ACCEPT TYPE IF NOT SUPPORTED isValid = false;
// 	}
// 	if (isValid == false) {
// 		_responseMessage.statusCode = "406";
// 		_responseMessage.statusMessage = "Not Acceptable";
// 		logger.log("ERROR", "checkAccept: Not Acceptable");
// 		result = generate400ErrorWebpage(_responseMessage, request);
// 		return result;
// 	}
// 	return result;
// }

// std::string checkAcceptEncoding(HttpRequest& request, Logger& logger, HttpResponseMessage& _responseMessage)
// {
// 	std::string result;
// 	bool isValid = false;
// 	std::string value = request.headers["Accept-Encoding"];
// 	std::istringstream iss(value);
// 	std::string token;
// 	std::vector<std::string> acceptEncodings;
// 	while (std::getline(iss, token, ',')) {
// 		token.erase(0, token.find_first_not_of(" "));
// 		acceptEncodings.push_back(token);
// 	}
// 	for (size_t i = 0; i < acceptEncodings.size(); i++) {
// 		std::string token2 = acceptEncodings[i];
// 		size_t semicolon = token2.find(';');
// 		if (semicolon != std::string::npos) {
// 			token2 = token2.substr(0, semicolon);
// 		}
// 		// CHECK THE ACCEPT ENCODING IF NOT SUPPORTED isValid = false;
// 		// KEYPOINT: WE ONLY NEED TO FIND OUT ANY ACCEPT ENCODING IS SUPPORTED.
// 		// IF ONE ACCEPT, THEN WE BREAK THE LOOP AND MARK THE VALUE TO THE CONTENT TYPE.
// 	}
// 	if (isValid == false) {
// 		_responseMessage.statusCode = "406";
// 		_responseMessage.statusMessage = "Not Acceptable";
// 		logger.log("ERROR", "checkAcceptEncoding: Not Acceptable");
// 		result = generate400ErrorWebpage(_responseMessage, request);
// 		return result;
// 	}
// 	return result;
// }

// void	checkAcceptLanguage(HttpRequest& request, Logger& logger, HttpResponseMessage& _responseMessage)
// {
// 	std::string result;
// 	bool isValid = false;
// 	std::string value = request.headers["Accept-Language"];
// 	std::istringstream iss(value);
// 	std::string token;
// 	std::vector<std::string> acceptLanguages;
// 	while (std::getline(iss, token, ',')) {
// 		token.erase(0, token.find_first_not_of(" "));
// 		acceptLanguages.push_back(token);
// 	}
// 	for (size_t i = 0; i < acceptLanguages.size(); i++) {
// 		std::string token2 = acceptLanguages[i];
// 		size_t semicolon = token2.find(';');
// 		if (semicolon != std::string::npos) {
// 			token2 = token2.substr(0, semicolon);
// 		}
// 		// CHECK THE ACCEPT LANGUAGE IF NOT SUPPORTED isValid = false;
// 		// KEYPOINT: WE ONLY NEED TO FIND OUT ANY ACCEPT LANGUAGE IS SUPPORTED.
// 		// IF ONE ACCEPT, THEN WE BREAK THE LOOP AND MARK THE VALUE TO THE CONTENT LANGUAGE.
// 	}
// 	if (isValid == false) {
// 		_responseMessage.contentLanguage = "en-US";
// 	}
// }

// std::string checkRange(HttpRequest& request, Logger& logger, HttpResponseMessage& _responseMessage)
// {
// 	std::string token = request.headers["Range"].substr(request.headers["Range"].find('=') + 1);
// 	std::istringstream iss(token);
// 	std::string range;
// 	std::vector<std::string> ranges;
// 	while (std::getline(iss, range, ',')) {
// 		range.erase(0, range.find_first_not_of(" "));
// 		ranges.push_back(range);
// 	}
// 	for (size_t i = 0; i < ranges.size(); i++) {
// 		int start = atoi(ranges[i].substr(0, ranges[i].find('-')).c_str());
// 		int end = atoi(ranges[i].substr(ranges[i].find('-') + 1).c_str());
// 		if (start > end) {
// 			_responseMessage.statusCode = "416";
// 			_responseMessage.statusMessage = "Requested Range Not Satisfiable";
// 			logger.log("ERROR", "checkRange: Requested Range Not Satisfiable");
// 			return generate400ErrorWebpage(_responseMessage, request);
// 		}
// 		if (start < 0 || end < 0) {
// 			_responseMessage.statusCode = "416";
// 			_responseMessage.statusMessage = "Requested Range Not Satisfiable";
// 			logger.log("ERROR", "checkRange: Requested Range Not Satisfiable");
// 			return generate400ErrorWebpage(_responseMessage, request);
// 		}
// 		std::ifstream file(request.uri.c_str());
// 		if (!file) {
// 			_responseMessage.statusCode = "404";
// 			_responseMessage.statusMessage = "Not Found";
// 			logger.log("ERROR", "checkRange: Not Found");
// 			return generate400ErrorWebpage(_responseMessage, request);
// 		}
// 		file.seekg(0, std::ios::end);
// 		int fileSize = file.tellg();
// 		if (end > fileSize || start > fileSize) {
// 			_responseMessage.statusCode = "416";
// 			_responseMessage.statusMessage = "Requested Range Not Satisfiable";
// 			logger.log("ERROR", "checkRange: Requested Range Not Satisfiable");
// 			return generate400ErrorWebpage(_responseMessage, request);
// 		}
// 		_responseMessage.isRange = true;
// 	}
// 	return std::string();
// }

// #include <sys/stat.h>
// #include <ctime>
// std::string checkIfModifiedSince(HttpRequest& request, Logger& logger, HttpResponseMessage& _responseMessage)
// {
// 	struct stat fileStat;
// 	std::string lastModifiedValue = request.headers["If-Modified-Since"];
// 	std::string lastModifiedTime;
//     if (stat(request.uri.c_str(), &fileStat) == 0) {
//         // Convert the last modified time to a readable format
//         char buffer[80];
//         struct tm* timeinfo = gmtime(&fileStat.st_mtime);  // Get the time in GMT
//         strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
//         lastModifiedTime = buffer;
//     }
// 	if (lastModifiedValue >= lastModifiedTime) {
// 		_responseMessage.statusCode = "304";
// 		_responseMessage.statusMessage = "Not Modified";
// 		logger.log("ERROR", "checkIfModifiedSince: Not Modified");
// 		return generate400ErrorWebpage(_responseMessage, request);
// 	} else {
// 		_responseMessage.statusCode = "200";
// 		_responseMessage.statusMessage = "OK";
// 		logger.log("INFO", "checkIfModifiedSince: OK");
// 		return std::string();
// 	}
// }

// #include <openssl/md5.h>
// #include <iomanip>
// std::string checkIfNoneMatch(HttpRequest& request, Logger& logger, HttpResponseMessage& _responseMessage)
// {
// 	std::string result;
// 	std::string ifNoneMatchValue = request.headers["If-None-Match"];
// 	std::string etagValue;
// 	std::ifstream file(request.uri, std::ios::binary);
//     if (!file) {
//         return "";
//     }

//     // Create an MD5 hash of the file content
//     std::ostringstream ss;
//     MD5_CTX md5Context;
//     MD5_Init(&md5Context);

//     // Read the file content and update the hash
//     char buffer[1024];
//     while (file.read(buffer, sizeof(buffer))) {
//         MD5_Update(&md5Context, buffer, file.gcount());
//     }
//     MD5_Update(&md5Context, buffer, file.gcount());

//     unsigned char hash[MD5_DIGEST_LENGTH];
//     MD5_Final(hash, &md5Context);

//     // Convert hash to a hex string
//     for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
//         ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
//     }
// 	etagValue = ss.str();
// 	if (ifNoneMatchValue == etagValue) {
// 		_responseMessage.statusCode = "304";
// 		_responseMessage.statusMessage = "Not Modified";
// 		logger.log("ERROR", "checkIfNoneMatch: Not Modified");
// 		result = generate400ErrorWebpage(_responseMessage, request);
// 		return result;
// 	} else {
// 		_responseMessage.statusCode = "200";
// 		_responseMessage.statusMessage = "OK";
// 		logger.log("INFO", "checkIfNoneMatch: OK");
// 		return result;
// 	}
// 	return result;
// }

// std::string checkContentType(HttpRequest& request, Logger& logger, HttpResponseMessage& _responseMessage)
// {
// 	std::string result;
// 	std::string contentType = request.headers["Content-Type"];
// 	// CHECK THE CONTENT TYPE IF NOT SUPPORTED
// 	// IF SUPPORTED, THEN WE NEED TO SET THE CONTENT TYPE INTO THE RESPONSE MESSAGE
// 	// else {
// 	// 	_responseMessage.statusCode = "415";
// 	// 	_responseMessage.statusMessage = "Unsupported Media Type";
// 	// 	logger.log("ERROR", "checkContentType: Unsupported Media Type");
// 	// 	result = generate400ErrorWebpage(_responseMessage, request);
// 	// 	return result;
// 	// }
// 	return result;
// }

// std::string checkContentLength(HttpRequest& request, Logger& logger, HttpResponseMessage& _responseMessage)
// {
// 	std::string result;
// 	std::string contentLength = request.headers["Content-Length"];
// 	if (contentLength.empty()) {
// 		_responseMessage.statusCode = "411";
// 		_responseMessage.statusMessage = "Length Required";
// 		logger.log("ERROR", "checkContentLength: Length Required");
// 		result = generate400ErrorWebpage(_responseMessage, request);
// 		return result;
// 	}
// 	if (atoi(contentLength.c_str()) < 0) {
// 		_responseMessage.statusCode = "400";
// 		_responseMessage.statusMessage = "Bad Request";
// 		logger.log("ERROR", "checkContentLength: Bad Request");
// 		result = generate400ErrorWebpage(_responseMessage, request);
// 		return result;
// 	}
// 	// CHECK THE BODY LENGTH
// 	// IF THE BODY LENGTH IS NOT EQUAL TO THE CONTENT LENGTH, THEN WE NEED TO RETURN 400 BAD REQUEST
// 	return result;
// }

// void checkTransferEncoding(Logger& logger, HttpResponseMessage& _responseMessage)
// {
// 	logger.log("INFO", "checkTransferEncoding: Transfer-Encoding: chunked");
// 	_responseMessage.isChunked = true;
// }

// std::string headerResponse(HttpRequest& request, Logger& logger, HttpResponseMessage& _responseMessage)
// {
// 	std::string result;
	
// 	for (std::map<std::string, std::string>::iterator it = request.headers.begin(); it != request.headers.end(); ++it) {
// 		if (it->first == "host") { //THE CONTENT "host" HERE REFER TO THE HOST NAME WE GOT FROM THE CONFIGURATION FILE
// 			if (!checkHost(request, logger, _responseMessage).empty()) {
// 				return checkHost(request, logger, _responseMessage);
// 			}
// 		}
// 		if (it->first == "connection") {
// 			checkConnection(request, logger, _responseMessage);
// 		}
// 		if (it->first == "upgrade") {
// 			;//WE DO NOT SUPPORT UPGRADE SO FAR BUT WE CAN ADD IT IF WE NEED TO
// 		}
// 		if (it->first == "accept") {
// 			if (!checkAccept(request, logger, _responseMessage).empty()) {
// 				return checkAccept(request, logger, _responseMessage);
// 			}
// 		}
// 		if (it->first == "accept-encoding") {
// 			if (!checkAcceptEncoding(request, logger, _responseMessage).empty()) {
// 				return checkAcceptEncoding(request, logger, _responseMessage);
// 			}
// 		}
// 		if (it->first == "accept-language") {
// 			checkAcceptLanguage(request, logger, _responseMessage);
// 		}
// 		if (it->first == "range") {
// 			if (!checkRange(request, logger, _responseMessage).empty()) {
// 				return checkRange(request, logger, _responseMessage);
// 			}
// 		}
// 		if (it->first == "if-modified-since") {
// 			if (!checkIfModifiedSince(request, logger, _responseMessage).empty()) {
// 				return checkIfModifiedSince(request, logger, _responseMessage);
// 			}
// 		}
// 		if (it->first == "if-none-match") {
// 			if (!checkIfNoneMatch(request, logger, _responseMessage).empty()) {
// 				return checkIfNoneMatch(request, logger, _responseMessage);
// 			}
// 		}
// 		if (it->first == "content-type") {
// 			if (!checkContentType(request, logger, _responseMessage).empty()) {
// 				return checkContentType(request, logger, _responseMessage);
// 			}
// 		}
// 		if (it->first == "content-length") {
// 			if (!checkContentLength(request, logger, _responseMessage).empty()) {
// 				return checkContentLength(request, logger, _responseMessage);
// 			}
// 		}
// 		if (it->first == "transfer-encoding") {
// 			checkTransferEncoding(logger, _responseMessage);
// 		}
// 		if (it->first == "expect") {
// 			;//WE DO NOT SUPPORT EXPECT SO FAR BUT WE CAN ADD IT IF WE NEED TO
// 		}
// 		if (it->first == "cookie") {
// 			;//WE DO NOT SUPPORT COOKIE SO FAR BUT WE CAN ADD IT IF WE NEED TO
// 		}
// 		// THE NEXT STEP IS TO GENERATE THE RESPONSE MESSAGE
// 	}
// }

// std::string toGet(HttpRequest& request, Logger& logger, HttpResponseMessage& _responseMessage)
// {
// 	std::string result;

// 	std::string uriPath = parseUri(request);
	
// 	result = headerResponse(request, logger, _responseMessage);

	
// }


// std::string HttpResponse::responToRequest(HttpRequest& request, Logger& logger)
// {
// 	std::string result;
// 	if (request.method == "GET")
// 	{
// 		result = toGet(request, logger, _responseMessage);
// 	}
// 	else if (request.method == "POST")
// 	{
		
// 	}
// 	else if (request.method == "PUT")
// 	{
		
// 	}
// 	else if (request.method == "DELETE")
// 	{
		
// 	}
// 	else if (request.method == "HEAD")
// 	{
		
// 	}
// 	else if (request.method == "OPTIONS")
// 	{
		
// 	}
// 	else
// 	{
		
// 	}
// }
