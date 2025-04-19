// #include "Logger.h"
// #include "Server.h"
// #include <iostream>
// #include <thread>

// int main() {
//     Logger logger;
//     Server svr(&logger);

//     std::thread serverThread = std::thread(&Server::start, &svr);
//     std::this_thread::sleep_for(std::chrono::milliseconds(5000));
//     svr.stop();
//     serverThread.join();
//     return 0;
// }
