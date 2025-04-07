1. What Are Sockets?
	* **Definitions:**
	  Sockets are endpoints for sending and receiving data across a network. It is like a communication channel between processes over the internet or local networks.
	* Types of Sockets (not all, only most useful):
		* Stream Sockets (TCP): Provide reliable. ordered communication.
		  Example: sending message (we can not loose any packets) 
		* Datagram Sockets (UDP): Provide connection-less, less reliable communication. 
		  Example: live streaming (speed over accuracy)
		* For HTTP server we will use TCP sockets. 

2. Basic Socket Functions
	 Creating a Socket
	* Function: socket()
		* Prototype:
			int socket(int domain, int type, int protocol);
		* Parameters:
			* domain: For IPv4 use AF_INET.
			* type: For a stream socket, use SOCK_STREAM.
			* protocol: Generally set to 0 to choose the default protocol (TCP for SOCK_STREAM)
		* Usage:
			int server_fd = socket(AF_INET, SOCKET_STREAM, 0);
			if (server_fd < 0)
				
	
