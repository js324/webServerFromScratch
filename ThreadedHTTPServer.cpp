/*
** server.c -- a stream socket server demo
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include <thread>
#include "threadpool.h"
#include "httpservers.h"

#define MAXDATASIZE 10000




class ThreadedHTTPServer: TCPServer {
private:
	ThreadPool pool;
	void serve_connection(int fd) {
		std::cout << "serving connection\n";
		std::cout.flush();
		while (1) {
			try {
				char buf[MAXDATASIZE];
				int numbytes;
				
				if ((numbytes = recv(fd, buf, MAXDATASIZE-1, 0)) == -1) {
					perror("recv");
					return;
				}
				buf[numbytes] = '\0';
				serve_request(std::string(buf, numbytes));
				printf("%s\n", buf);
				if (numbytes == 0) {
					std::cout << "Connection closed!\n";
					return;
				}
			}
			catch (...) {
				std::cout << "Connection failure!";
			}
		}
}
public:
	//default const
	ThreadedHTTPServer(std::string ip, int port): TCPServer(ip, port) {}
	void runServer() {
		pool.startUp();
		int sockfd = bindAndListen();
		struct sockaddr_storage their_addr; // connector's address information
        socklen_t sin_size;
		while(1) {  // main accept() loop
			sin_size = sizeof their_addr;
			int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
			if (new_fd == -1) {
				perror("accept");
				continue;
			}

			// inet_ntop(their_addr.ss_family,
			// 	get_in_addr((struct sockaddr *)&their_addr),
			// 	s, sizeof s);
			// printf("server: got connection from %s\n", s);
			
			pool.queueJob([new_fd, this]() { this->serve_connection(new_fd); });
		}
	}
	void closeServer() {
		pool.close();
	}
};
int main(void)
{
	ThreadedHTTPServer server("localhost", 3490);
	server.runServer();	
	

	return 0;
}
