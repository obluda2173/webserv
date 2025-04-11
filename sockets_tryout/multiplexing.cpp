#include <vector> // For dynamic management of pollfd

void fun() {
    std::vector<struct pollfd> pfds;
    pfds.push_back({_server_fd, POLLIN, 0}); // Initial file descriptor setup to listen for new connections

    while (true) {
        int ready = poll(pfds.data(), pfds.size(), 100);

        if (ready == -1)
            errExit("poll");

        for (size_t i = 0; i < pfds.size(); i++) {
            if (pfds[i].revents & POLLIN) {
                if (pfds[i].fd == _server_fd) {
                    // Ready to accept a new connection
                    int new_socket = accept(_server_fd, (struct sockaddr *)&_address, &_addrlen);

                    if (new_socket == -1)
                        errExit("accept");

                    struct pollfd new_pfd = {new_socket, POLLIN, 0};
                    pfds.push_back(new_pfd);
                } else {
                    char buffer[BUFFER_SIZE];
                    ssize_t bytes_read = read(pfds[i].fd, buffer, sizeof(buffer));
                    if (bytes_read > 0) {
                        // Process data, prepare response
                        // Here you would parse HTTP, determine response, etc.

                        // Example: Mark socket to watch for writing
                        pfds[i].events = pfds[i].events | POLLOUT; // Add POLLOUT to event flags
                    } else {
                        // Handle disconnection
                        close(pfds[i].fd);
                        pfds.erase(pfds.begin() + i);
                        i--; // Adjust loop index after removal
                    }
                }
            }

            if (pfds[i].revents & POLLOUT) {
                const char *response = "HTTP/1.1 200 OK\r\nContent-Length: "
                                       "13\r\n\r\nHello, world!";
                ssize_t bytes_sent = send(pfds[i].fd, response, strlen(response), 0);

                if (bytes_sent > 0) {
                    // Remove POLLOUT event after sending response
                    pfds[i].events &= ~POLLOUT;

                    // Optionally close and remove socket if done
                    close(pfds[i].fd);
                    pfds.erase(pfds.begin() + i);
                    i--; // Adjust loop index after removal
                }
            }
        }

        // Optionally, manage pfds (remove closed connections)
    }
}
