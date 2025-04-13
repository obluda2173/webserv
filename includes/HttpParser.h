#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <IHttpParser.h>


class HttpParser : public IHttpParser {
	private:
	HttpRequest _req;
	// do whatever you want inside private, but don't change public. (period)
	public:
		~HttpParser() = default;
		void parse(char* buf); // !!! buffer needs to be NULL-terminated !!!
		int error(void);
		int ready(void);
		HttpRequest getRequest(void);
	};

// HttpRequest	HttpParser::parse(const std::string& request)
// {
// 	HttpRequest store = new HttpRequest();
// }

#endif // HTTPPARSER_H
