#include "Socket.h"

int main() {
	// Logger* logger = new Logger("log.txt");
	Logger* logger = new Logger();

	try {
		Server server(80, logger);
		server.listen();
	} catch (...) {
		exit(EXIT_FAILURE);
	}
	delete logger;
}
