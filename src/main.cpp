#include "Socket.h"

int main() {
	Logger* logger = new Logger("log.txt");
	try {
		Server server(80, logger);
		server.listen();
	} catch (...) {
		exit(EXIT_FAILURE);
	}
	delete logger;
}
