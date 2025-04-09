#include "Socket.h"

int main() {

	try {
		Server server;
		server.listen();
	} catch (...) {
		exit(EXIT_FAILURE);
	}
}
