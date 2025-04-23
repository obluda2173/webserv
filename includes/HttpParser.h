#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <IHttpParser.h>
#include <Logger.h>
#include <cstring>
#include <string.h>

class HttpParser : public IHttpParser {
  private:
    enum State { STATE_REQUEST_LINE, STATE_HEADERS, STATE_DONE, STATE_ERROR };

    State _state;
    std::string _buffer;
    HttpRequest _currentRequest;
    size_t _totalProcessedSize;
    size_t _maxHeaderKeySize; // 256B
    size_t _maxHeaderSize;    // 4KB
    ILogger* _logger;
    bool _extractNextLine(std::string& line);
    void _parseRequestLine(const std::string& line);
    void _parseHeader(const std::string& line);
    void _handlePartialLine();
    void _reset();
    void _parseBuffer();
    bool _requestLineValidation(
        const std::string& method,
        const std::string& version); // because we don't need to check uri, so I don't put it in the function
    bool _headerLineValidation(const std::string& key, const std::string& value);

  public:
    HttpParser(ILogger* logger);
    HttpParser(size_t maxHeaderKeySize, size_t maxHeaderSize, ILogger* logger);
    ~HttpParser();
    void feed(const char* buffer, size_t length);
    int error(void);
    int ready(void);
    HttpRequest getRequest(void);
};

#endif // HTTPPARSER_H
