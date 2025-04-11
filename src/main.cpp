#include "Server.h"

int main() {
    // Logger* logger = new Logger("log.txt");
    Logger *logger = new Logger();

    // TODO: parse the config file
    // construct config Class
    // config.parse()
    try {
        Server server(logger);
        // TODO: registering Handlers depending on the config
        server.listen(8080);
        server.handleConnections();
    } catch (const std::exception &e) {
        logger->log("ERROR", "Exception caught: " + std::string(e.what()));
        delete logger;
        exit(EXIT_FAILURE);
    }
    delete logger;
}
