#ifndef	HTTPRESPONSE_H
#define	HTTPRESPONSE_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <hash_map>

#include <Logger.h>
#include <HttpParser.h>

//THIS IS BAD TO DO IT HERE, BUT THE BETTER WAY IS TO DO IT IN THE SERVER CLASS BECAUSE WE NEED SOME OF THE DATA IN THE SERVER CLASS

typedef struct HttpResponse
{
	std::string statusCode;
	std::string statusMessage;
	bool isClosed = false;
	std::string contentType;
	std::string contentLength;
	std::string contentLanguage;
	bool isRange = false;
} HttpResponseMessage;



class HttpResponse
{
private:
	HttpResponseMessage _responseMessage;
public:
	std::string responToRequest(HttpRequest& request, Logger& logger);

	//so we need to deal with the header first and then the content
	//amybe we need to go through the map and then find out the how to deal with each specific header
	//and then we need to deal with the content

	//1. find the uri and then find the Host and its value, find the value inside the uri, if not found, then we need to add the value infront of the uri, to make it absolute path
	//2. check the value of the cache-control.
};


#endif 