#include "Socket.h"

int main() {
	// Logger* logger = new Logger("log.txt");
	Logger* logger = new Logger();

	try {
		Server server(80, logger);

		if (server.listen() < 0) {
			logger->log("ERROR", "Failed to start server");
			delete logger;
			exit(EXIT_FAILURE);
		}
		server.handleConnections();
	} catch (const std::exception& e) {
		logger->log("ERROR", "Exception caught: " + std::string(e.what()));
		delete logger;
		exit(EXIT_FAILURE);
	}
	delete logger;
}