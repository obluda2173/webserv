#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <IHttpParser.h>
#include <string.h>

class HttpParser : public IHttpParser {
  private:
		HttpRequest _req;
		bool	_isReady;
    bool	_isFullHeader;
		bool	_isMethodReady;
		bool	_isError;
		void	findMethod(char* buf, int& i);
    // do whatever you want inside private, but don't change public. (period)

  public:
    ~HttpParser() = default;
    void parse(char* buf); // !!! buffer needs to be NULL-terminated !!!
    int error(void);
    int ready(void);
    HttpRequest getRequest(void);
};

void	HttpParser::findMethod(char* buf, int& i)
{
	if (_isMethodReady == true)
		return ;
	while (buf[i] != '\0' && buf[i] != ' ')
	{
		_req.method.push_back(buf[i]);
		i++;
	}
	if (buf[i] == ' ' && (_req.method == "GET" || _req.method == "DELETE" || _req.method == "POST")) {
		_isMethodReady = true;
		return ;
	}
	else if (buf[i] == ' ') {
		_isError = true;
		return ;
	}
	else
		return ;
}

void  HttpParser::parse(char *buf)
{
	_isMethodReady = false;
	char* endHeader = strstr(buf, "\r\n\r\n");
	if (endHeader == NULL) {
		_isFullHeader = false;
	} else {
		_isFullHeader = true;
	}
	
	int i = 0;
	while ((endHeader - buf[i]) && buf[i] != '\0') {
		// if (findMethod(buf, i) && (_req.method == "GET" || _req.method == "DELETE" || _req.method == "POST"))
		// 	_isError = false;
		
	}
	
  // loop for body (optional)
  // use content length
}

#endif // HTTPPARSER_H
