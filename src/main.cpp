#include "Server.h"

int main() {
	// Logger* logger = new Logger("log.txt");
	Logger* logger = new Logger();

	try {
		Server server(logger);
		server.listen(80);
		server.handleConnections();
	} catch (const std::exception& e) {
		logger->log("ERROR", "Exception caught: " + std::string(e.what()));
		delete logger;
		exit(EXIT_FAILURE);
	}
	delete logger;
}
