1. **GENERAL INFORMATION**
	* **Definitions:**
	  Sockets are endpoints for sending and receiving data across a network. It is like a communication channel between processes over the internet or local networks.
	* Types of Sockets (not all, only most useful):
		* Stream Sockets (TCP): Provide reliable. ordered communication.
		  Example: sending message (we can not loose any packets) 
		* Datagram Sockets (UDP): Provide connection-less, less reliable communication. 
		  Example: live streaming (speed over accuracy)
		* For HTTP server we will use TCP sockets. 

2. **TECHNICAL ASPECTS AND IMPLEMENTATION**
	 Creating a Socket
	* Function: socket()
		* Prototype:
			```cpp
			int socket(int domain, int type, int protocol);
			```
		* Parameters:
			* domain: For IPv4 use AF_INET.
			* type: For a stream socket, use SOCK_STREAM.
			* protocol: Generally set to 0 to choose the default protocol (TCP for SOCK_STREAM)
		* Usage:
			```cpp
			int server_fd = socket(AF_INET, SOCKET_STREAM, 0);
			if (server_fd < 0)
				// handle error
			```
	Binding the Socket
	* Function: bind()
		* Purpose: Associates your socket with a specific IP address and port.
		* Prototype:
			int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
		* Usage:
			* Define a sockaddr_in structure:
				```cpp
				struct sockaddr_in structure:
				address.sin_family = AF_NET;
				address.sin_addr.s_addr = INADDR_ANY;
				address.sin_port = htons(port_number);
				```
			* Then bind:
				 ```cpp
				 if (bind(server_fd, (struct sockaddr \*)&address), sizeof(address) < 0)
					// handle error
				```
	Listening for Connections
	* Function: listen()
		* Purpose: Tells the socket to listen for incoming connections.
		* Prototype:
			```cpp
			int listen(int sockfd, int backlog);
			```
		* Backlog: The number of connections that can be queued.
		* Usage:
			```cpp
			if (listen(server_fd, SOMAXCONN) < 0)
				// handle error
			```
	Accepting Connections
	* Function: accept()
		* Purpose: Accept an incoming connection, creating a new socket for the client.
		* Prototype:
			```cpp
			int accept(int sockfd, struct sockaddr \*addr, socklen_t \*addrlen);
			```
		* Usage:
			```cpp
			struct sockaddr_in client_address;
			socklen_t addrlen = sizeof(client_address);
			int client_fd = accept(server_fd, (struct sockaddr \*)&client_address, &addrlen);
			if (client_fd < 0)
				// handle error
			```
	* Note:
	  For non-blocking servers, accept() is called only when it is known there is an incoming connection (e.g. using poll())

3. **DATA TRANSMISSION (MORE TECHNICAL ASPECTS)** 
	Sending Data
	* Function: send()
		* Usage:
			```cpp
			ssize_t sent_bytes = send(client_fd, data, data_length, 0)
			if (sent_bytes < 0)
				// handle error
			```
	Receiving Data
	* Function: recv()
		* Usage:
			```cpp
			char buffer[1024];
			ssize_t received_bytes = recv(client_fd, buffer, sizeof(buffer), 0);
			if (recaived_bytes < 0)
				// handle error
			```
	Important:
	Always check that you are ready to read/write via your polling mechanism before calling these functions to avoid blocking.

4. **SOCKET OPTIONS**
	* Function: setsockopt()
		* Purpose: Modify options at the socket level (e.g., allowing address reuse)
		* Common Option: So_REUSEADDR
		* Usage:
		```cpp
			int opt = 1;
			if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
				// handle error
		```
5. **NON-BLOCKING MODE**
	* Why Non-Blocking?
		* webserv must not block on any single operation. Non-blocking mode allows to poll multiple sockets simultaneously without waiting on one call to complete.
	* Setting Non-Blocking Mode:
		* Using fcntl()
			```cpp
			#include <fcntl>
			
			int flags = fcntl(server_fd, F_GETFL, 0);
			if (flag == -1)
				// handle error
			if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) == -1)
				// handle error
			```
	* Using in I/O Loop:
		With non-blocking sockets, you check if data is available for reading or if the socket is ready for writing using a multiplexing mechanism like poll().

6.  **ERROR HANDLING**
	* Common Error Checks:
		After each socket operation, check the return value for errors.
		Note: For the webserv I must not rely on checking errno after read/write calls; instead, use the return values provided by my polling mechanism.
	* Graceful Shutdown:
		Make sure to close sockets using close() when done, and clean up any child process (if using cgi with fork())

7.  **INTEGRATING SOCKETS WITH AN EVENT LOOP**
	* I/O Multiplexing:
		Use one of the following to monitor multiple sockets:
		* select()
		  Suitable for smaller numbers of file descriptors.
		* poll()
		  More scalable and easier to use than select().
		* epoll()/kqueue()
		  For high-performance need.
	* Event Loop Essentials:
		* Monitor the listening socket for new connections.
		* Monitor client sockets for readability (incoming data) and writability (ready to send data).
		* Handle state transitions (e.g., connections accepted, data received, response sent).

8. **SUMMARY FOR WEBSERV**
	* Initialisation:
		Create, configure (using setsockopt()), bind, and listen on your sockets.
	* Main loop:
		Use an event loop (poll/select/epoll/kqueue) to:
		* Accept new client connections.
		* Handle incoming requests.
		* Send out HTTP responses.
	* Non-Blocking Design:
		Ensure every socket is set to non-blocking mode, and all I/O operations are guarded by your event loop.
	* Error Handling & Cleanup:
		Robustly manage errors and ensure resources are cleaned up properly (close sockets, manage child processes).
