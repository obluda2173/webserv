#include "ILogger.h"

class Server {
private:
	ILogger* _logger;
	bool _isRunning;
public:
	explicit Server(ILogger* l);
	bool isRunning() const;
	void start();
};
